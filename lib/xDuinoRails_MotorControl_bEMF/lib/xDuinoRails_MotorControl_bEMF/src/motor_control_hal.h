/**
 * @file motor_control_hal.h
 * @brief Hardware Abstraction Layer (HAL) for low-level motor control.
 *
 * This file defines a platform-agnostic interface for hardware-accelerated
 * PWM motor control and BEMF (Back-EMF) measurement. The implementation for
 * a specific microcontroller must be provided separately.
 */
#ifndef MOTOR_CONTROL_HAL_H
#define MOTOR_CONTROL_HAL_H

#include <cstdint>

// PWM frequency for the motor driver
const uint32_t PWM_FREQUENCY_HZ = 20000;

// Delay after the PWM cycle before triggering ADC, allows the motor coils' magnetic field to collapse.
const uint32_t BEMF_MEASUREMENT_DELAY_US = 10;
// Ring buffer size for ADC samples. Must be a power of 2 for DMA efficiency on some platforms.
const uint32_t BEMF_RING_BUFFER_SIZE = 64;

// Default Shunt Resistor value in Ohms (can be overridden by build flags)
#ifndef SHUNT_RESISTOR_OHMS
#define SHUNT_RESISTOR_OHMS 0.5f
#endif

// Default Short Circuit Current Limit in Amps (can be overridden)
#ifndef MAX_CURRENT_AMPS
#define MAX_CURRENT_AMPS 2.0f
#endif

/**
 * @brief Callback function pointer type for BEMF updates.
 *
 * This function is called from an interrupt context whenever a new
 * differential BEMF measurement is available from the hardware. It is the
 * responsibility of the callee to perform any necessary filtering, processing,
 * and control logic adjustments.
 *
 * @param raw_bemf_value The raw, unfiltered differential BEMF value, calculated
 *                       as the absolute difference between the two ADC readings.
 */
typedef void (*hal_bemf_update_callback_t)(int raw_bemf_value);

/**
 * @brief Initializes the low-level hardware for motor control.
 *
 * This function configures the hardware timers, PWM peripherals, ADC, and DMA
 * for hardware-accelerated motor control and BEMF measurement. It must be
 * called once during the application's setup phase.
 *
 * @param pwm_a_pin The GPIO pin number for PWM channel A (e.g., forward).
 * @param pwm_b_pin The GPIO pin number for PWM channel B (e.g., reverse).
 * @param bemf_a_pin The GPIO pin number for ADC input connected to motor terminal A.
 * @param bemf_b_pin The GPIO pin number for ADC input connected to motor terminal B.
 * @param callback A pointer to a function that will be called from an interrupt
 *                 context with new BEMF data.
 */
void hal_motor_init(uint8_t pwm_a_pin, uint8_t pwm_b_pin, uint8_t bemf_a_pin, uint8_t bemf_b_pin, hal_bemf_update_callback_t callback);

/**
 * @brief Sets the motor's PWM duty cycle and direction.
 *
 * This function updates the PWM hardware with the new duty cycle. It should
 * be called periodically from the main application loop to reflect the latest
 * output from the motor control algorithm (e.g., a PI controller).
 *
 * @param duty_cycle The desired duty cycle, typically in a range from 0 to 255.
 * @param forward The desired motor direction (true for forward, false for reverse).
 */
void hal_motor_set_pwm(int duty_cycle, bool forward);

/**
 * @brief Retrieves the BEMF ring buffer for diagnostics.
 *
 * This function provides low-level access to the raw ADC sample buffer.
 * It is intended for debugging and visualization, not for real-time control.
 * The buffer contains interleaved samples from ADC A and ADC B.
 *
 * @param[out] buffer A pointer to a uint16_t pointer that will be set to the
 *                    address of the internal ring buffer.
 * @param[out] last_write_pos A pointer to an integer that will be set to the
 *                            last written position in the buffer.
 * @return The total size of the ring buffer (number of samples).
 */
int hal_motor_get_bemf_buffer(volatile uint16_t** buffer, int* last_write_pos);

/**
 * @brief Retrieves the Current Sensing ring buffer.
 *
 * Optional: Only available if enabled/supported by the platform and configuration.
 *
 * @param[out] buffer Pointer to the uint16_t pointer that will be set to the address of the buffer.
 * @param[out] last_write_pos Pointer to an integer that will be set to the last written position.
 * @return The size of the ring buffer, or 0 if not supported.
 */
int hal_motor_get_current_buffer(volatile uint16_t** buffer, int* last_write_pos);

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
void hal_motor_check_solenoid_position(int ping_pwm_value, int ping_duration_ms, int measurement_delay_us, int* response_a, int* response_b);

#endif // MOTOR_CONTROL_HAL_H
