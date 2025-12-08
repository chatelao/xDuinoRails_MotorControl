#include "motor_control_hal.h"
#include <Arduino.h>

/**
 * @brief Checks the solenoid/motor position by pinging both directions and measuring the response.
 *
 * This function performs a diagnostic test:
 * 1. Pings the "forward" direction (PWM A).
 * 2. Measures the BEMF/Response immediately after switch-off.
 * 3. Pings the "reverse" direction (PWM B).
 * 4. Measures the BEMF/Response immediately after switch-off.
 *
 * @param ping_pwm_value The PWM duty cycle (0-255) for the ping pulse.
 * @param ping_duration_ms The duration of the ping pulse in milliseconds.
 * @param measurement_delay_us The delay after switching off before measuring the response.
 * @param[out] response_a The measured response for the forward/A direction.
 * @param[out] response_b The measured response for the reverse/B direction.
 */
void hal_motor_check_solenoid_position(int ping_pwm_value, int ping_duration_ms, int measurement_delay_us, int* response_a, int* response_b) {
    // We need to capture the BEMF value. The HAL usually provides it via callback or buffer.
    // Since this is a synchronous diagnostic function, we can't easily rely on the asynchronous callback
    // unless we hook it or poll the buffer.
    // Polling the buffer is safer as it doesn't require modifying the global callback state which might be in use.

    volatile uint16_t* buffer;
    int last_write_pos;
    int buffer_size;

    // --- Ping A (Forward) ---
    hal_motor_set_pwm(ping_pwm_value, true);
    delay(ping_duration_ms);
    hal_motor_set_pwm(0, true); // Switch off

    // Wait for the measurement delay.
    // Note: The HAL typically handles BEMF measurement automatically during the OFF phase.
    // However, if we set PWM to 0, the "OFF phase" is continuous.
    // Some HALs (like RP2040 implementation) might skip measurement if duty is 0 or if the calculated delay is weird.
    // But usually, if we just switched off, the motor current is decaying.
    // The HAL's continuous background process should capture this if it's running.
    // IF the HAL relies on PWM triggers, and PWM is 0 (constant low), triggers might stop or be different.

    // Crucial assumption: The HAL continues to update the BEMF buffer even when PWM is 0 (or we just switched to 0).
    // In RP2040 HAL: "if (trigger_tick_pos >= PWM_WRAP_VALUE) { g_skip_measurement = true; }"
    // If PWM is 0, level is 0. trigger_tick_pos = 0 + settling_ticks.
    // If settling_ticks < PWM_WRAP_VALUE, it measures!
    // So for low duty (or 0), it SHOULD measure.

    delayMicroseconds(measurement_delay_us);

    // Read the latest value from the buffer
    buffer_size = hal_motor_get_bemf_buffer(&buffer, &last_write_pos);
    if (buffer_size > 0 && buffer != nullptr) {
        // Get the last written sample.
        // last_write_pos points to the *next* write position (or the current write address).
        // So we want the previous one.
        int index = (last_write_pos - 1 + buffer_size) % buffer_size;
        // The buffer contains interleaved data [B, A, B, A...].
        // We want the magnitude.
        // But wait, the buffer contains RAW ADC values.
        // We want the difference or the specific channel?
        // The RP2040 HAL callback calculates `abs(sum_A - sum_B)`.
        // The buffer has [B, A, B, A].
        // index might be A or B.
        // Let's grab the last few samples and calculate the difference.

        // Let's take the last pair.
        // Ensure index is even aligned or we know the order.
        // RP2040: "De-interleave ... for (uint i = 0; i < BEMF_RING_BUFFER_SIZE; i += 2) { sum_B += buf[i]; sum_A += buf[i+1]; }"
        // So Even indices (0, 2...) are B. Odd indices (1, 3...) are A.

        // We want to sample the response.
        // Let's Average the entire buffer? No, that might contain old data.
        // We just waited `measurement_delay_us`.
        // The buffer is constantly being overwritten by DMA.
        // We should just take the latest valid pair.

        int idx_a, idx_b;
        if (index % 2 != 0) { // index is Odd (A)
            idx_a = index;
            idx_b = (index - 1 + buffer_size) % buffer_size;
        } else { // index is Even (B)
             idx_b = index;
             idx_a = (index - 1 + buffer_size) % buffer_size;
        }

        // Calculate differential
        int val_a = buffer[idx_a];
        int val_b = buffer[idx_b];
        *response_a = abs(val_a - val_b);

    } else {
        *response_a = -1; // Error or not supported
    }

    // Short pause between pings to let things settle
    delay(50);

    // --- Ping B (Reverse) ---
    hal_motor_set_pwm(ping_pwm_value, false);
    delay(ping_duration_ms);
    hal_motor_set_pwm(0, false); // Switch off

    delayMicroseconds(measurement_delay_us);

    buffer_size = hal_motor_get_bemf_buffer(&buffer, &last_write_pos);
    if (buffer_size > 0 && buffer != nullptr) {
        int index = (last_write_pos - 1 + buffer_size) % buffer_size;
        int idx_a, idx_b;
        if (index % 2 != 0) { // index is Odd (A)
            idx_a = index;
            idx_b = (index - 1 + buffer_size) % buffer_size;
        } else { // index is Even (B)
             idx_b = index;
             idx_a = (index - 1 + buffer_size) % buffer_size;
        }

        int val_a = buffer[idx_a];
        int val_b = buffer[idx_b];
        *response_b = abs(val_a - val_b);
    } else {
        *response_b = -1;
    }

    // Ensure motor is off
    hal_motor_set_pwm(0, true);
}
