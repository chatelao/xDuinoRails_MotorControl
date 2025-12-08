/**
 * @file motor_control_hal_esp32.cpp
 * @brief ESP32-specific implementation for the motor control HAL.
 */

#include "motor_control_hal.h"

#if defined(ARDUINO_ARCH_ESP32)

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
#include "driver/adc.h"
#else
#include "driver/adc_deprecated.h"
#endif

#include <Arduino.h>
#include "driver/mcpwm_prelude.h"
#include "driver/gptimer.h"
#include "soc/soc_caps.h"
#include "driver/adc.h"
#include "soc/adc_channel.h"
#include "hal/adc_ll.h"
#include "esp_private/adc_private.h"
#include "hal/dma_types.h"
#include "soc/i2s_struct.h"

// Timer resolution for PWM
const uint32_t PWM_TIMER_RESOLUTION_HZ = 10 * 1000 * 1000; // 10 MHz

// --- ADC DMA Configuration ---
// The ADC is configured to run in a continuous DMA mode. This means the hardware
// will automatically sample the BEMF pins and store the results in a buffer
// without any CPU intervention.

// Each ADC result is 2 bytes long.
#define ADC_RESULT_BYTE 2
// We define a "frame" as a set of readings from all BEMF channels (in our case, 2).
// The DMA buffer will hold 100 such frames.
#define ADC_CONV_FRAME_SIZE 100
// The total size of the DMA buffer in bytes.
#define ADC_DMA_BUF_SIZE (ADC_CONV_FRAME_SIZE * ADC_RESULT_BYTE)

// --- Static Globals for Hardware Control ---

// Callback function to the main driver to report the measured BEMF value.
static hal_bemf_update_callback_t bemf_callback = nullptr;

// Pin definitions and ADC channel mappings.
static uint8_t g_pwm_a_pin;
static uint8_t g_pwm_b_pin;
static uint8_t g_bemf_a_pin;
static uint8_t g_bemf_b_pin;
static adc_channel_t g_bemf_a_channel;
static adc_channel_t g_bemf_b_channel;

// The DMA buffer where the ADC results will be stored.
// Must be volatile as it's written by DMA hardware.
static volatile uint16_t s_dma_buf[ADC_DMA_BUF_SIZE] = {0};
// Public buffer for BEMF diagnostics (strictly 2 channel layout)
static volatile uint16_t s_bemf_user_buf[ADC_CONV_FRAME_SIZE] = {0};
static int s_bemf_user_buf_pos = 0;

// Handles for the various ESP32 hardware peripherals we are using.
static mcpwm_cmpr_handle_t comparator_a = nullptr;
static mcpwm_cmpr_handle_t comparator_b = nullptr;
static mcpwm_oper_handle_t oper = nullptr;
static mcpwm_timer_handle_t timer = nullptr;
static gptimer_handle_t gptimer = nullptr;

void hal_read_and_process_bemf();

void hal_motor_init(uint8_t pwm_a_pin, uint8_t pwm_b_pin, uint8_t bemf_a_pin, uint8_t bemf_b_pin, hal_bemf_update_callback_t callback) {
    g_pwm_a_pin = pwm_a_pin;
    g_pwm_b_pin = pwm_b_pin;
    g_bemf_a_pin = bemf_a_pin;
    g_bemf_b_pin = bemf_b_pin;
    bemf_callback = callback;

    // --- MCPWM (Motor Control PWM) Setup ---
    // The MCPWM peripheral is a powerful PWM generator designed for motor control.
    // We configure it to generate the PWM signals for our motor driver.
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = PWM_TIMER_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = PWM_TIMER_RESOLUTION_HZ / PWM_FREQUENCY_HZ,
    };
    mcpwm_new_timer(&timer_config, &timer);

    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
    };
    mcpwm_new_operator(&operator_config, &oper);
    mcpwm_operator_connect_timer(oper, timer);

    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    mcpwm_new_comparator(oper, &comparator_config, &comparator_a);
    mcpwm_new_comparator(oper, &comparator_config, &comparator_b);

    mcpwm_generator_handle_t generator_a = nullptr;
    mcpwm_generator_config_t gen_config_a = { .gen_gpio_num = g_pwm_a_pin };
    mcpwm_new_generator(oper, &gen_config_a, &generator_a);

    mcpwm_generator_handle_t generator_b = nullptr;
    mcpwm_generator_config_t gen_config_b = { .gen_gpio_num = g_pwm_b_pin };
    mcpwm_new_generator(oper, &gen_config_b, &generator_b);

    // Set actions for generators to produce standard PWM
    mcpwm_generator_set_action_on_timer_event(generator_a, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    mcpwm_generator_set_action_on_compare_event(generator_a, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_a, MCPWM_GEN_ACTION_LOW));
    mcpwm_generator_set_action_on_timer_event(generator_b, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
    mcpwm_generator_set_action_on_compare_event(generator_b, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_b, MCPWM_GEN_ACTION_LOW));

    mcpwm_timer_enable(timer);
    mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);

    // --- GPTimer Setup for BEMF Measurement Delay ---
    // After the PWM pulse ends, we need to wait a short time for the motor coils
    // to stabilize before we can get a clean BEMF reading. We use a high-precision
    // General Purpose Timer (GPTimer) for this.
    gptimer_config_t gptimer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
    };
    gptimer_new_timer(&gptimer_config, &gptimer);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = BEMF_MEASUREMENT_DELAY_US,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false,
    };
    gptimer_set_alarm_action(gptimer, &alarm_config);
    gptimer_enable(gptimer);
    // The timer is not started here. It will be triggered automatically by the ETM.

    bemf_callback = callback;

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    adc_digi_initialize(NULL);
#else
    adc_digi_init();
#endif

    // --- ADC Digital Controller (DMA Mode) Setup ---
    g_bemf_a_channel = (adc_channel_t)ADC1_GPIO32_CHANNEL;
    g_bemf_b_channel = (adc_channel_t)ADC1_GPIO33_CHANNEL;

    adc1_config_channel_atten(g_bemf_a_channel, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(g_bemf_b_channel, ADC_ATTEN_DB_11);

#if defined(MOTOR_CURRENT_PIN)
    // 3 Channels: A, B, Current
    #define NUM_CHANNELS 3
    adc_channel_t current_channel = ADC1_GPIO34_CHANNEL; // Map MOTOR_CURRENT_PIN manually if possible or assume defined
    // Note: Standard Arduino ESP32 doesn't easily map Pin -> Channel Enum.
    // We assume GPIO34 (ADC1_6) for devkit if not specified.
    // Ideally use digitalPinToAnalogChannel?
    // digitalPinToAnalogChannel(MOTOR_CURRENT_PIN) returns 0..7 for ADC1?
    // Let's assume user defines correct PIN. But we need channel enum.
    // For now, we reuse the pattern structure logic.
#else
    // 2 Channels: A, B
    #define NUM_CHANNELS 2
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    adc_digi_pattern_config_t adc_pattern[NUM_CHANNELS] = {
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = g_bemf_a_channel,
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = g_bemf_b_channel,
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
#if defined(MOTOR_CURRENT_PIN)
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = (adc_channel_t)digitalPinToAnalogChannel(MOTOR_CURRENT_PIN),
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
#endif
    };

    adc_digi_configuration_t dig_cfg = {
        .conv_limit_en = false,
        .conv_limit_num = 255,
        .pattern_num = sizeof(adc_pattern) / sizeof(adc_digi_pattern_config_t),
        .adc_pattern = adc_pattern,
        .sample_freq_hz = 20000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };
    adc_digi_controller_configure(&dig_cfg);
#else
    adc_digi_pattern_table_t adc_pattern[NUM_CHANNELS] = {
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = g_bemf_a_channel,
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = g_bemf_b_channel,
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
#if defined(MOTOR_CURRENT_PIN)
        {
            .atten = ADC_ATTEN_DB_11,
            .channel = (adc_channel_t)digitalPinToAnalogChannel(MOTOR_CURRENT_PIN),
            .unit = ADC_UNIT_1,
            .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
        },
#endif
    };

    adc_digi_config_t dig_cfg = {
        .conv_limit_en = false,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .adc1_pattern_len = sizeof(adc_pattern) / sizeof(adc_digi_pattern_table_t),
        .adc1_pattern = adc_pattern,
        .dma_eof_num = ADC_CONV_FRAME_SIZE,
        .dma_work_mode = ADC_DMA_WORK_MODE_CIRCULAR,
        .dma_data = (void*)s_dma_buf,
        .dma_size = ADC_DMA_BUF_SIZE,
    };
    adc_digi_controller_config(&dig_cfg);
#endif
    adc_digi_start();


    // --- ETM (Event Task Matrix) Setup for Hardware Triggering ---
    // The ETM is the magic that connects all our hardware peripherals together.
    // It allows us to create a chain of events and tasks that run without any
    // CPU intervention.
    //
    // Our trigger chain works like this:
    // 1. The MCPWM timer reaches its peak (end of PWM cycle). This is our main timing event.
    // 2. The ETM connects this MCPWM "timer full" event to a GPTimer "start" task.
    // 3. The GPTimer starts counting for the 10us BEMF delay.
    // 4. When the GPTimer alarm fires (after 10us), it triggers an ISR.
    // 5. The ISR then programmatically starts the ADC DMA conversion.

    // Create an ETM event for the MCPWM timer reaching its peak.
    mcpwm_etm_event_config_t mcpwm_event_config = { .event_type = MCPWM_TIMER_EVENT_FULL };
    esp_etm_event_handle_t mcpwm_timer_event;
    mcpwm_new_etm_event(timer, &mcpwm_event_config, &mcpwm_timer_event);

    // Create an ETM task to start the GPTimer.
    gptimer_etm_task_config_t gptimer_start_task_config = { .task_type = GPTIMER_ETM_TASK_START_COUNT };
    esp_etm_task_handle_t gptimer_start_task;
    gptimer_new_etm_task(gptimer, &gptimer_start_task_config, &gptimer_start_task);

    gptimer_etm_event_config_t gptimer_event_config = { .event_type = GPTIMER_EVENT_ALARM };
    esp_etm_event_handle_t gptimer_alarm_event;
    gptimer_new_etm_event(gptimer, &gptimer_event_config, &gptimer_alarm_event);

    // As noted, a direct ETM task for the ADC digital controller is not yet
    // available in the driver. So, we fall back to a minimal ISR.
    gptimer_event_callbacks_t cbs = {
        .on_alarm = gptimer_alarm_isr_handler,
    };
    gptimer_register_event_callbacks(gptimer, &cbs, nullptr);

    // Create a task to stop the GPTimer after it has fired.
    gptimer_etm_task_config_t gptimer_stop_task_config = { .task_type = GPTIMER_ETM_TASK_STOP_COUNT };
    esp_etm_task_handle_t gptimer_stop_task;
    gptimer_new_etm_task(gptimer, &gptimer_stop_task_config, &gptimer_stop_task);

    // Connect the MCPWM "timer full" event to the GPTimer "start" task.
    esp_etm_channel_config_t etm_config_1 = {};
    esp_etm_channel_handle_t etm_channel_1;
    esp_etm_new_channel(&etm_config_1, &etm_channel_1);
    esp_etm_channel_connect(etm_channel_1, mcpwm_timer_event, gptimer_start_task);
    esp_etm_channel_enable(etm_channel_1);

    // Connect the GPTimer "alarm" event to the GPTimer "stop" task.
    // This makes the timer a "one-shot" timer that is re-triggered on every PWM cycle.
    esp_etm_channel_config_t etm_config_3 = {};
    esp_etm_channel_handle_t etm_channel_3;
    esp_etm_new_channel(&etm_config_3, &etm_channel_3);
    esp_etm_channel_connect(etm_channel_3, gptimer_alarm_event, gptimer_stop_task);
    esp_etm_channel_enable(etm_channel_3);
}

/**
 * @brief ISR that is triggered by the GPTimer alarm after the 10us delay.
 *
 * This function is designed to be as fast as possible. Its only jobs are to
 * start the ADC DMA conversion and reset the timer for the next cycle.
 */
static bool IRAM_ATTR gptimer_alarm_isr_handler(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
    // Trigger the ADC digital controller to start a new conversion sequence.
    adc_digi_start();
    // Reset the timer's counter to 0 for the next ETM trigger.
    gptimer_set_raw_count(timer, 0);
    return false;
}

/**
 * @brief Reads and processes BEMF data from the DMA buffer.
 *
 * This function is called from the main `update()` loop of the motor driver.
 * It checks for new data in the circular DMA buffer, reads it, processes it,
 * and sends the calculated BEMF value to the main driver via the callback.
 */
void hal_read_and_process_bemf() {
    uint8_t results[ADC_CONV_FRAME_SIZE];
    uint32_t ret_num = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    esp_err_t result = adc_digi_read_bytes(results, ADC_CONV_FRAME_SIZE, &ret_num, 0);
#else
    uint8_t result = adc_digi_read_bytes(results, ADC_CONV_FRAME_SIZE, &ret_num, 0);
#endif

    if (result == ESP_OK && ret_num > 0) {
        adc_digi_output_data_t* p_data = (adc_digi_output_data_t*)results;
        int num_samples = ret_num / sizeof(adc_digi_output_data_t);

        // Loop stride depends on channel count (2 or 3)
        // If 3 channels, [A, B, I, A, B, I...]
        // Note: The driver returns data in order of pattern.
#if defined(MOTOR_CURRENT_PIN)
        int stride = 3;
#else
        int stride = 2;
#endif

        for (int i = 0; i < num_samples; i += stride) {
            if (i + stride > num_samples) break;

            uint16_t adc_val_a = p_data[i].type2.data;
            uint16_t adc_val_b = p_data[i+1].type2.data;

#if defined(MOTOR_CURRENT_PIN)
            uint16_t adc_val_current = p_data[i+2].type2.data;
            // FAST SHUTDOWN CHECK
            const float V_limit = MAX_CURRENT_AMPS * SHUNT_RESISTOR_OHMS;
            const uint16_t adc_threshold = (uint16_t)((V_limit / 3.3f) * 4095.0f);
            if (adc_val_current > adc_threshold) {
                mcpwm_comparator_set_compare_value(comparator_a, 0);
                mcpwm_comparator_set_compare_value(comparator_b, 0);
            }
#endif

            // Store BEMF in user buffer (Always 2 channels)
            s_bemf_user_buf[s_bemf_user_buf_pos] = adc_val_a;
            s_bemf_user_buf_pos = (s_bemf_user_buf_pos + 1) % ADC_CONV_FRAME_SIZE;
            s_bemf_user_buf[s_bemf_user_buf_pos] = adc_val_b;
            s_bemf_user_buf_pos = (s_bemf_user_buf_pos + 1) % ADC_CONV_FRAME_SIZE;

            if (bemf_callback) {
                bemf_callback(abs(adc_val_a - adc_val_b));
            }
        }
    }
}

static int get_dma_write_pos() {
    // The ADC DMA on ESP32 uses the I2S0 peripheral. We can get the DMA
    // descriptor address to find the current write position.
    dma_descriptor_t* desc = (dma_descriptor_t*) I2S0.in_suc_eof_desc_addr;
    // The descriptor's buffer pointer gives us the address where the DMA last wrote.
    uintptr_t current_write_addr = (uintptr_t)desc->buffer;
    uintptr_t buffer_start_addr = (uintptr_t)s_dma_buf;
    int byte_offset = current_write_addr - buffer_start_addr;

    // The DMA buffer is circular. If we have a negative offset, it means the
    // write pointer has wrapped around, or we're at the beginning of the chain.
    // We need to find the descriptor that is currently pointing into our buffer.
    if (byte_offset < 0 || byte_offset > ADC_DMA_BUF_SIZE) {
        // This is a simplified approach. A more robust solution might be needed
        // if the DMA controller is using a complex descriptor chain.
        // For now, we assume the next descriptor is the one we want.
        desc = (dma_descriptor_t*)desc->next;
        current_write_addr = (uintptr_t)desc->buffer;
        byte_offset = current_write_addr - buffer_start_addr;
    }

    // Convert byte offset to sample index.
    return byte_offset / sizeof(uint16_t);
}

int hal_motor_get_bemf_buffer(volatile uint16_t** buffer, int* last_write_pos) {
    *buffer = s_bemf_user_buf;
    *last_write_pos = s_bemf_user_buf_pos;
    return ADC_CONV_FRAME_SIZE;
}

void hal_motor_set_pwm(int duty_cycle, bool forward) {
    uint32_t duty_ticks = (PWM_TIMER_RESOLUTION_HZ / PWM_FREQUENCY_HZ) * (duty_cycle / 255.0f);

    if (forward) {
        mcpwm_comparator_set_compare_value(comparator_a, duty_ticks);
        mcpwm_comparator_set_compare_value(comparator_b, 0);
    } else {
        mcpwm_comparator_set_compare_value(comparator_a, 0);
        mcpwm_comparator_set_compare_value(comparator_b, duty_ticks);
    }
}

#endif // ARDUINO_ARCH_ESP32
