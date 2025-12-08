/**
 * @file motor_control_hal_generic.cpp
 * @brief Generic Arduino implementation for the motor control HAL.
 *
 * This implementation uses standard Arduino functions (analogWrite, digitalRead, etc.)
 * and provides a virtual BEMF feedback based on the set speed.
 */

#include "motor_control_hal.h"
#include <Arduino.h>

// Guard to ensure this is only used if no specific architecture is selected
#if !defined(ARDUINO_ARCH_ESP32) && \
    !defined(ARDUINO_ARCH_NRF52) && \
    !defined(ARDUINO_ARCH_RP2040) && \
    !defined(ARDUINO_ARCH_SAMD) && \
    !defined(ARDUINO_ARCH_STM32)

static uint8_t g_pwm_a_pin;
static uint8_t g_pwm_b_pin;
static uint8_t g_bemf_a_pin;
static uint8_t g_bemf_b_pin;
static hal_bemf_update_callback_t g_bemf_callback = nullptr;

// Simulated buffer
static volatile uint16_t s_bemf_buffer[2];

void hal_motor_init(uint8_t pwm_a_pin, uint8_t pwm_b_pin, uint8_t bemf_a_pin, uint8_t bemf_b_pin, hal_bemf_update_callback_t callback) {
    g_pwm_a_pin = pwm_a_pin;
    g_pwm_b_pin = pwm_b_pin;
    g_bemf_a_pin = bemf_a_pin;
    g_bemf_b_pin = bemf_b_pin;
    g_bemf_callback = callback;

    pinMode(g_pwm_a_pin, OUTPUT);
    pinMode(g_pwm_b_pin, OUTPUT);
    // BEMF pins are usually analog inputs, which don't strictly require pinMode(INPUT),
    // but it's good practice on some platforms.
    pinMode(g_bemf_a_pin, INPUT);
    pinMode(g_bemf_b_pin, INPUT);

    analogWrite(g_pwm_a_pin, 0);
    analogWrite(g_pwm_b_pin, 0);
}

void hal_motor_set_pwm(int duty_cycle, bool forward) {
    if (forward) {
        analogWrite(g_pwm_a_pin, duty_cycle);
        analogWrite(g_pwm_b_pin, 0);
    } else {
        analogWrite(g_pwm_a_pin, 0);
        analogWrite(g_pwm_b_pin, duty_cycle);
    }

    // Simulate BEMF:
    // Virtual BEMF is roughly proportional to the duty cycle (speed).
    // The BEMF value in other HALs is typically the ADC difference.
    // Let's scale it to a typical 12-bit ADC range (0-4095) for realism.
    // 255 (max duty) -> ~3000 (arbitrary max BEMF).
    int virtual_bemf = map(duty_cycle, 0, 255, 0, 3000);

    // Update the simulated buffer so diagnostic functions have something to read
    // Buffer layout is interleaved [B, A].
    // If forward (A active), BEMF is on B?
    // Actually, usually BEMF is measured on the OFF pin or via differential.
    // Let's just put the virtual value in the buffer.
    if (forward) {
         s_bemf_buffer[0] = 0;
         s_bemf_buffer[1] = virtual_bemf;
    } else {
         s_bemf_buffer[0] = virtual_bemf;
         s_bemf_buffer[1] = 0;
    }

    // Software Short Circuit Protection
#if defined(MOTOR_CURRENT_PIN)
    // Threshold calculation: V = I * R
    // V_adc = (V / 3.3) * 1023 (Arduino default) or 4095
    // Assuming standard 10-bit resolution for Generic analogRead
    const float limit_volts = MAX_CURRENT_AMPS * SHUNT_RESISTOR_OHMS;
    const int limit_adc = (int)((limit_volts / 3.3f) * 1023.0f);

    if (analogRead(MOTOR_CURRENT_PIN) > limit_adc) {
        analogWrite(g_pwm_a_pin, 0);
        analogWrite(g_pwm_b_pin, 0);
        // Do not update callback or continue
        return;
    }
#endif

    if (g_bemf_callback) {
        g_bemf_callback(virtual_bemf);
    }
}

int hal_motor_get_bemf_buffer(volatile uint16_t** buffer, int* last_write_pos) {
    *buffer = s_bemf_buffer;
    *last_write_pos = 0;
    return 2;
}

int hal_motor_get_current_buffer(volatile uint16_t** buffer, int* last_write_pos) {
    *buffer = nullptr;
    *last_write_pos = 0;
    return 0;
}

#endif
