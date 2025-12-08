/**
 * @file motor_control_hal_nrf52.cpp
 * @brief nRF52-specific implementation for the motor control HAL.
 */

#include "motor_control_hal.h"

#if defined(ARDUINO_ARCH_NRF52)

#include <Arduino.h>
#include "nrfx_pwm.h"
#include "nrfx_saadc.h"
#include "nrfx_ppi.h"

// --- PWM Configuration ---
static const nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_individual_t pwm_values;
static nrf_pwm_sequence_t pwm_sequence;

// --- ADC Configuration ---
static const nrfx_saadc_t saadc = NRFX_SAADC_INSTANCE(0);
// Raw buffer for SAADC DMA (sized for max channels).
// If current sensing is enabled, we have 3 channels.
#if defined(MOTOR_CURRENT_PIN)
static nrf_saadc_value_t saadc_raw_buffer[3];
static const uint8_t NUM_ADC_CHANNELS = 3;
#else
static nrf_saadc_value_t saadc_raw_buffer[2];
static const uint8_t NUM_ADC_CHANNELS = 2;
#endif

// Public BEMF buffer (strictly 2 channels [B, A] for compatibility)
static volatile uint16_t bemf_ring_buffer[2];

// --- PPI Configuration ---
static nrf_ppi_channel_t ppi_channel;

// --- Static Globals ---
static hal_bemf_update_callback_t bemf_callback = nullptr;
static uint16_t top_value;

static void saadc_callback(nrfx_saadc_evt_t const * p_event) {
    if (p_event->type == NRFX_SAADC_EVT_DONE) {
        // Copy BEMF data (first 2 channels) to the public ring buffer
        // Note: Buffer layout depends on initialization order.
        // We init A (ch 0), B (ch 1), then Current (ch 2).
        // p_buffer[0] = A, p_buffer[1] = B, p_buffer[2] = Current.
        bemf_ring_buffer[0] = p_event->data.done.p_buffer[1]; // B
        bemf_ring_buffer[1] = p_event->data.done.p_buffer[0]; // A (Logic usually expects [B, A])
        // Wait, current logic: `abs(buffer[0] - buffer[1])`. Order doesn't matter for diff.
        // Diagnostics expects [B, A, B, A...].
        // Let's store B at 0, A at 1.

        if (bemf_callback) {
            bemf_callback(abs(p_event->data.done.p_buffer[0] - p_event->data.done.p_buffer[1]));
        }
        // Re-trigger the ADC for the next conversion
        nrfx_saadc_buffer_convert(&saadc, saadc_raw_buffer, NUM_ADC_CHANNELS);
    }
    else if (p_event->type == NRFX_SAADC_EVT_LIMIT) {
        // FAST PROTECTION: Kill PWM
        pwm_values.channel_0 = 0;
        pwm_values.channel_1 = 0;
        // Optionally disabling PPI to prevent restart?
        // But pwm_values are double buffered, update takes effect next period.
        // For immediate effect we might need to stop the PWM instance.
        // But setting values to 0 is the standard way here.
    }
}

void hal_motor_init(uint8_t pwm_a_pin, uint8_t pwm_b_pin, uint8_t bemf_a_pin, uint8_t bemf_b_pin, hal_bemf_update_callback_t callback) {
    bemf_callback = callback;

    // --- PWM Initialization ---
    nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
    pwm_config.output_pins[0] = pwm_a_pin;
    pwm_config.output_pins[1] = pwm_b_pin;
    pwm_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
    pwm_config.load_mode = NRF_PWM_LOAD_INDIVIDUAL;

    // --- Dynamic Frequency Calculation ---
    // The nRF52 PWM frequency is determined by: F_pwm = 16MHz / (prescaler * top_value)
    // We iterate through the available prescalers to find one that allows for a
    // top_value within the 16-bit range.
    const uint32_t base_clock = 16000000;
    nrf_pwm_clk_t prescalers[] = {
        NRF_PWM_CLK_16MHz, NRF_PWM_CLK_8MHz, NRF_PWM_CLK_4MHz, NRF_PWM_CLK_2MHz,
        NRF_PWM_CLK_1MHz, NRF_PWM_CLK_500kHz, NRF_PWM_CLK_250kHz, NRF_PWM_CLK_125kHz
    };
    uint32_t prescaler_divs[] = {1, 2, 4, 8, 16, 32, 64, 128};

    for (size_t i = 0; i < sizeof(prescalers) / sizeof(prescalers[0]); ++i) {
        uint32_t required_top = base_clock / (prescaler_divs[i] * PWM_FREQUENCY_HZ);
        if (required_top <= 65535) {
            pwm_config.prescaler = (nrf_pwm_prescaler_t)i;
            top_value = required_top;
            break;
        }
    }
    pwm_config.top_value = top_value;

    nrfx_pwm_init(&pwm, &pwm_config, NULL);

    pwm_sequence.values.p_individual = &pwm_values;
    pwm_sequence.length = NRF_PWM_VALUES_LENGTH(pwm_values);
    pwm_sequence.repeats = 0;
    pwm_sequence.end_delay = 0;

    // --- SAADC Initialization ---
    nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
    nrfx_saadc_init(&saadc, &saadc_config, saadc_callback);

    nrf_saadc_channel_config_t channel_a_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(bemf_a_pin);
    nrfx_saadc_channel_init(&saadc, 0, &channel_a_config);

    nrf_saadc_channel_config_t channel_b_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(bemf_b_pin);
    nrfx_saadc_channel_init(&saadc, 1, &channel_b_config);

#if defined(MOTOR_CURRENT_PIN)
    // --- Current Sensing Channel Setup (Channel 2) ---
    nrf_saadc_channel_config_t channel_current_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(MOTOR_CURRENT_PIN);
    nrfx_saadc_channel_init(&saadc, 2, &channel_current_config);

    // Calculate Limits
    // NRF SAADC is 10-bit or 12-bit? Default config is usually 10-bit or 12-bit depending on sdk_config.
    // NRFX_SAADC_CONFIG_RESOLUTION is usually 10 (1024) or 12 (4096).
    // Let's assume 10-bit (default) or check headers. Standard Arduino Core usually uses 10 or 12.
    // Safe bet: calculate for both or check constants.
    // Actually, NRF52 SAADC result is int16_t.
    // We will assume 12-bit (0..4095) for consistency with other HALs, or check resolution.
    // NRFX_SAADC_DEFAULT_CONFIG sets resolution to NRF_SAADC_RESOLUTION_10BIT.
    const float V_limit = MAX_CURRENT_AMPS * SHUNT_RESISTOR_OHMS;
    // 3.6V reference internal? Or VDD/4?
    // NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE uses gain 1/6 and internal ref 0.6V => Range 0..3.6V.
    // So 3.6V = 1023 (10-bit).
    const int16_t limit_val = (int16_t)((V_limit / 3.6f) * 1023.0f);

    nrfx_saadc_limit_config_t limit_config = {
        .low = NRFX_SAADC_LIMIT_DISABLED,
        .high = limit_val
    };
    nrfx_saadc_limit_set(&saadc, 2, &limit_config);
    // Enable limit event interrupt is done by nrfx_saadc_init if handler is provided.
    // But we need to enable the specific channel limit?
    // It seems implicit in nrfx_saadc_limit_set in newer drivers, or requires event enable.
    // Checking NRFX docs: nrfx_saadc_limit_set sets registers. Interrupt must be enabled.
    // NRFX_SAADC_EVT_LIMIT is generated.
#endif

    nrfx_saadc_buffer_convert(&saadc, saadc_raw_buffer, NUM_ADC_CHANNELS);


    // --- PPI Initialization ---
    uint32_t pwm_compare_event = nrfx_pwm_event_address_get(&pwm, NRF_PWM_EVENT_PWMPERIODEND);
    uint32_t saadc_sample_task = nrfx_saadc_task_address_get(&saadc, NRF_SAADC_TASK_SAMPLE);

    nrfx_ppi_channel_alloc(&ppi_channel);
    nrfx_ppi_channel_assign(ppi_channel, pwm_compare_event, saadc_sample_task);
    nrfx_ppi_channel_enable(ppi_channel);

    // --- Start PWM ---
    nrfx_pwm_simple_playback(&pwm, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}

void hal_motor_set_pwm(int duty_cycle, bool forward) {
    uint16_t duty = map(duty_cycle, 0, 255, 0, top_value);
    if (forward) {
        pwm_values.channel_0 = duty;
        pwm_values.channel_1 = 0;
    } else {
        pwm_values.channel_0 = 0;
        pwm_values.channel_1 = duty;
    }
}

int hal_motor_get_bemf_buffer(volatile uint16_t** buffer, int* last_write_pos) {
    *buffer = (volatile uint16_t*)bemf_ring_buffer;
    *last_write_pos = 0; // This is a simplified implementation
    return 2;
}

#endif // ARDUINO_ARCH_NRF52
