#include "motor_control_hal.h"

#if defined(ARDUINO_ARCH_STM32)

#include <Arduino.h>

// Static Globals
static hal_bemf_update_callback_t bemf_callback = nullptr;
static uint8_t g_bemf_a_pin;
static uint8_t g_bemf_b_pin;
static uint8_t g_pwm_a_pin;
static uint8_t g_pwm_b_pin;
static volatile uint16_t bemf_ring_buffer[BEMF_RING_BUFFER_SIZE];

// HardwareTimer
HardwareTimer* pwm_timer;
uint32_t pwm_channel_a;
uint32_t pwm_channel_b;

// DMA and ADC handles
DMA_HandleTypeDef hdma_adc;
ADC_HandleTypeDef hadc;

#if defined(STM32G4)
// OpAmp Handles for G4
OPAMP_HandleTypeDef hopamp1;
OPAMP_HandleTypeDef hopamp3;

#if defined(MOTOR_CURRENT_PIN) || defined(ENABLE_CURRENT_SENSING)
// Shared ADC2 Handle for protection or measurement
ADC_HandleTypeDef hadc2;
// Hardware Protection Handles (DAC + COMP)
DAC_HandleTypeDef hdac1;
COMP_HandleTypeDef hcomp2;
#endif

#if defined(DEBUG_WITH_DAC)
    #if defined(MOTOR_CURRENT_PIN) || defined(ENABLE_CURRENT_SENSING)
        // Reuse hdac1 from protection
        #define DEBUG_DAC_HANDLE (&hdac1)
    #else
        // Create dedicated handle
        DAC_HandleTypeDef hdac_debug;
        #define DEBUG_DAC_HANDLE (&hdac_debug)
    #endif
#endif

#if defined(ENABLE_CURRENT_SENSING)
OPAMP_HandleTypeDef hopamp2;
DMA_HandleTypeDef hdma_adc2;
static volatile uint16_t current_ring_buffer[BEMF_RING_BUFFER_SIZE];

extern "C" void DMA1_Channel2_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc2);
}
#endif

// ADC IRQ Handler (Shared for ADC1 and ADC2 on G4)
extern "C" void ADC1_2_IRQHandler(void) {
    #if defined(MOTOR_CURRENT_PIN) || defined(ENABLE_CURRENT_SENSING)
    HAL_ADC_IRQHandler(&hadc2);
    #endif
    HAL_ADC_IRQHandler(&hadc);
}

// DMA IRQ Handler for G4 (Assuming DMA1 Channel 1)
extern "C" void DMA1_Channel1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc);
}
#else
// DMA IRQ Handler for F4 (ADC1 is on DMA2 Stream 0)
extern "C" void DMA2_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc);
}

// ADC IRQ Handler for F4 (For Analog Watchdog)
extern "C" void ADC_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc);
}
#endif

// ADC Watchdog Callback for Short Circuit Protection
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc) {
    // FAST SHUTDOWN: Disable PWM immediately
    if (pwm_timer) {
        pwm_timer->pause();
        pwm_timer->setCaptureCompare(pwm_channel_a, 0, TICK_COMPARE_FORMAT);
        pwm_timer->setCaptureCompare(pwm_channel_b, 0, TICK_COMPARE_FORMAT);
    }
}

// Shared Process Function
static void process_bemf_data(volatile uint16_t* buffer, uint32_t length) {
    uint32_t sum_A = 0, sum_B = 0;
    for (uint32_t i = 0; i < length; i += 2) {
        sum_A += buffer[i];
        sum_B += buffer[i + 1];
    }
    int measured_bemf = abs((int)(sum_A / (length / 2)) - (int)(sum_B / (length / 2)));

#if defined(DEBUG_WITH_DAC) && defined(STM32G4)
    // Write to DAC (12-bit)
    uint32_t dac_val = (uint32_t)measured_bemf;
    if(dac_val > 4095) dac_val = 4095;
    HAL_DAC_SetValue(DEBUG_DAC_HANDLE, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_val);
#endif

    if (bemf_callback) {
        bemf_callback(measured_bemf);
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    process_bemf_data(&bemf_ring_buffer[BEMF_RING_BUFFER_SIZE / 2], BEMF_RING_BUFFER_SIZE / 2);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    process_bemf_data(bemf_ring_buffer, BEMF_RING_BUFFER_SIZE / 2);
}

void hal_motor_init(uint8_t pwm_a_pin, uint8_t pwm_b_pin, uint8_t bemf_a_pin, uint8_t bemf_b_pin, hal_bemf_update_callback_t callback) {
    g_pwm_a_pin = pwm_a_pin;
    g_pwm_b_pin = pwm_b_pin;
    g_bemf_a_pin = bemf_a_pin;
    g_bemf_b_pin = bemf_b_pin;
    bemf_callback = callback;

    // --- PWM Setup ---
    TIM_TypeDef *pwm_timer_instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(g_pwm_a_pin), PinMap_PWM);
    pwm_timer = new HardwareTimer(pwm_timer_instance);

    pwm_channel_a = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(g_pwm_a_pin), PinMap_PWM));
    pwm_channel_b = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(g_pwm_b_pin), PinMap_PWM));

    pwm_timer->setMode(pwm_channel_a, TIMER_OUTPUT_COMPARE_PWM1, g_pwm_a_pin);
    pwm_timer->setMode(pwm_channel_b, TIMER_OUTPUT_COMPARE_PWM1, g_pwm_b_pin);

    pwm_timer->setOverflow(PWM_FREQUENCY_HZ, HERTZ_FORMAT);

#if defined(STM32G4)
    // --- G4 Specific Setup ---

    // Enable Clocks
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_RCC_OPAMP_CLK_ENABLE();
    __HAL_RCC_DAC1_CLK_ENABLE();
    __HAL_RCC_COMP_CLK_ENABLE();

    // Configure OpAmps (Follower Mode)
    // OPAMP1 (Connected to PA1)
    hopamp1.Instance = OPAMP1;
    hopamp1.Init.PowerMode = OPAMP_POWERMODE_NORMAL;
    hopamp1.Init.Mode = OPAMP_FOLLOWER_MODE;
    hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0; // PA1
    hopamp1.Init.InternalOutput = DISABLE;
    hopamp1.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
    hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
    HAL_OPAMP_Init(&hopamp1);
    HAL_OPAMP_Start(&hopamp1);

    // OPAMP3 (Connected to PB0)
    hopamp3.Instance = OPAMP3;
    hopamp3.Init.PowerMode = OPAMP_POWERMODE_NORMAL;
    hopamp3.Init.Mode = OPAMP_FOLLOWER_MODE;
    hopamp3.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0; // PB0
    hopamp3.Init.InternalOutput = DISABLE;
    hopamp3.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
    hopamp3.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
    HAL_OPAMP_Init(&hopamp3);
    HAL_OPAMP_Start(&hopamp3);

#if defined(MOTOR_CURRENT_PIN) || defined(ENABLE_CURRENT_SENSING)
    // Calculate Threshold
    // Threshold = (I_max * R_shunt / 3.3V) * 4095
    const float V_limit = MAX_CURRENT_AMPS * SHUNT_RESISTOR_OHMS;
    const uint32_t adc_threshold = (uint32_t)((V_limit / 3.3f) * 4095.0f);

    // --- HARDWARE PROTECTION: DAC + COMP + TIM_BREAK ---

    // 1. Configure DAC to generate threshold voltage
    hdac1.Instance = DAC1;
    HAL_DAC_Init(&hdac1);
    DAC_ChannelConfTypeDef sConfigDAC = {0};
    sConfigDAC.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
    sConfigDAC.DAC_DMADoubleDataMode = DISABLE;
    sConfigDAC.DAC_SignedFormat = DISABLE;
    sConfigDAC.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    sConfigDAC.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfigDAC.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    sConfigDAC.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE; // Connect to COMP
    sConfigDAC.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
    HAL_DAC_ConfigChannel(&hdac1, &sConfigDAC, DAC_CHANNEL_1);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, adc_threshold);

    // 2. Configure Comparator (COMP2)
    // Input Plus: OPAMP2 Output (Internal)
    // Input Minus: DAC1 Channel 1 (Internal)
    hcomp2.Instance = COMP2;
    hcomp2.Init.InputPlus = COMP_INPUT_PLUS_OPAMP2;
    hcomp2.Init.InputMinus = COMP_INPUT_MINUS_DAC1_CH1;
    hcomp2.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp2.Init.Hysteresis = COMP_HYSTERESIS_LOW;
    hcomp2.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp2.Init.TriggerMode = COMP_TRIGGERMODE_NONE; // We use BKIN, not IT
    HAL_COMP_Init(&hcomp2);
    HAL_COMP_Start(&hcomp2);

    // 3. Configure Timer Break (Hardware Shutdown)
    // Link COMP2 Output to TIM1 Break Input
    TIM_BreakDeadTimeConfigTypeDef sBreakConfig = {0};
    sBreakConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakConfig.DeadTime = 0;
    sBreakConfig.BreakState = TIM_BREAK_ENABLE;
    sBreakConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH; // Active High from COMP
    sBreakConfig.BreakFilter = 0;
    sBreakConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE; // Restart after break clears? Or DISABLE to stay off?
    // Safety: DISABLE automatic restart usually preferred for short circuit
    sBreakConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

    // Enable Break Input from COMP2
    HAL_TIMEx_ConfigBreakDeadTime(pwm_timer->getHandle(), &sBreakConfig);

    TIM_Ex_BreakInputConfigTypeDef sBreakInputConfig = {0};
    sBreakInputConfig.Source = TIM_BREAKINPUTSOURCE_COMP2;
    sBreakInputConfig.Enable = TIM_BREAKINPUTSOURCE_ENABLE;
    sBreakInputConfig.Polarity = TIM_BREAKINPUTSOURCE_POLARITY_HIGH;
    HAL_TIMEx_ConfigBreakInput(pwm_timer->getHandle(), TIM_BREAKINPUT_BRK, &sBreakInputConfig);

#if defined(ENABLE_CURRENT_SENSING)
    // If ENABLE_CURRENT_SENSING is defined, we use OPAMP2/ADC2 setup below.
    // We just need to add the Analog Watchdog to it (Redundant but good telemetry).
    // OPAMP2 (Connected to PA7/D11)
    hopamp2.Instance = OPAMP2;
    hopamp2.Init.PowerMode = OPAMP_POWERMODE_NORMAL;
    hopamp2.Init.Mode = OPAMP_FOLLOWER_MODE;
    hopamp2.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0; // PA7
    hopamp2.Init.InternalOutput = ENABLE; // Must enable internal output for COMP?
                                          // Note: COMP_INPUT_PLUS_OPAMP2 usually works with internal link.
                                          // However, if we drive external ADC pin too, we need both?
                                          // G4 OPAMP InternalOutput usually refers to ADC channel.
                                          // Checking: OPAMP output to ADC is dedicated channel.
                                          // OPAMP output to pin is "Standalone" or "Follower".
                                          // Let's set InternalOutput DISABLE as standard unless we know we need it for COMP.
                                          // Actually, COMP takes from 'Internal node'.
    hopamp2.Init.InternalOutput = DISABLE; // Standard pin output for ADC2
    hopamp2.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
    hopamp2.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
    HAL_OPAMP_Init(&hopamp2);
    HAL_OPAMP_Start(&hopamp2);

    // Configure ADC2
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE; // Single channel
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO; // Same trigger
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.NbrOfConversion = 1;
    hadc2.Init.DMAContinuousRequests = ENABLE;
    hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc2.Init.OversamplingMode = DISABLE;
    HAL_ADC_Init(&hadc2);

    ADC_ChannelConfTypeDef sConfig2 = {0};
    sConfig2.Channel = ADC_CHANNEL_3; // PA6 (OPAMP2 Out)
    sConfig2.Rank = ADC_REGULAR_RANK_1;
    sConfig2.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig2.SingleDiff = ADC_SINGLE_ENDED;
    sConfig2.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig2);

    // Configure Analog Watchdog for Short Circuit Protection
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
    AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
    AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
    AnalogWDGConfig.Channel = ADC_CHANNEL_3; // Monitor Current Channel
    AnalogWDGConfig.ITMode = ENABLE;
    AnalogWDGConfig.HighThreshold = adc_threshold;
    AnalogWDGConfig.LowThreshold = 0;
    HAL_ADC_AnalogWDGConfig(&hadc2, &AnalogWDGConfig);

    // Configure DMA (DMA1 Channel 2 for ADC2)
    hdma_adc2.Instance = DMA1_Channel2;
    hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&hdma_adc2);

    __HAL_LINKDMA(&hadc2, DMA_Handle, hdma_adc2);

    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    // Enable ADC IRQ for Watchdog
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);

    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t*)current_ring_buffer, BEMF_RING_BUFFER_SIZE);
#elif defined(MOTOR_CURRENT_PIN)
    // Case: MOTOR_CURRENT_PIN is defined but ENABLE_CURRENT_SENSING is NOT.
    // We need to set up a dedicated ADC (ADC2) just for protection, without the ring buffer.

    // Configure ADC2
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    // We can use software start or TRGO. TRGO matches PWM cycle (good for noise)
    hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO;
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.NbrOfConversion = 1;
    hadc2.Init.DMAContinuousRequests = DISABLE; // No DMA needed for protection only
    hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc2.Init.OversamplingMode = DISABLE;
    HAL_ADC_Init(&hadc2);

    // Find channel for MOTOR_CURRENT_PIN
    PinName p = digitalPinToPinName(MOTOR_CURRENT_PIN);
    uint32_t channel = STM_PIN_CHANNEL(pinmap_function(p, PinMap_ADC));

    ADC_ChannelConfTypeDef sConfig2 = {0};
    sConfig2.Channel = channel;
    sConfig2.Rank = ADC_REGULAR_RANK_1;
    sConfig2.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig2.SingleDiff = ADC_SINGLE_ENDED;
    sConfig2.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig2);

    // Configure Analog Watchdog
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
    AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1;
    AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
    AnalogWDGConfig.Channel = channel;
    AnalogWDGConfig.ITMode = ENABLE;
    AnalogWDGConfig.HighThreshold = adc_threshold;
    AnalogWDGConfig.LowThreshold = 0;
    HAL_ADC_AnalogWDGConfig(&hadc2, &AnalogWDGConfig);

    // Enable ADC IRQ
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);

    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADC_Start(&hadc2); // Start conversion (triggered by Timer or SW)
#endif
#endif

#if defined(DEBUG_WITH_DAC)
    // --- DEBUG DAC SETUP ---
    #if !defined(MOTOR_CURRENT_PIN) && !defined(ENABLE_CURRENT_SENSING)
        // Initialize DAC if not done by protection
        __HAL_RCC_DAC1_CLK_ENABLE();
        hdac_debug.Instance = DAC1;
        HAL_DAC_Init(&hdac_debug);
    #endif

    // Configure Channel 2 (PA5) for Debug Output
    // Ensure PA5 is in Analog Mode
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    DAC_ChannelConfTypeDef sConfigDebug = {0};
    sConfigDebug.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
    sConfigDebug.DAC_DMADoubleDataMode = DISABLE;
    sConfigDebug.DAC_SignedFormat = DISABLE;
    sConfigDebug.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
    sConfigDebug.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfigDebug.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    sConfigDebug.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
    sConfigDebug.DAC_UserTrimming = DAC_TRIMMING_FACTORY;

    HAL_DAC_ConfigChannel(DEBUG_DAC_HANDLE, &sConfigDebug, DAC_CHANNEL_2);
    HAL_DAC_Start(DEBUG_DAC_HANDLE, DAC_CHANNEL_2);
#endif

    // Configure ADC1
    // ADC1 IN3 is OPAMP1 Output (PA2)
    // ADC1 IN12 is OPAMP3 Output (PB1)

    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc.Init.ContinuousConvMode = DISABLE; // Triggered by Timer
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T1_TRGO; // Trigger from Timer 1
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 2;
    hadc.Init.DMAContinuousRequests = ENABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc.Init.OversamplingMode = DISABLE;
    HAL_ADC_Init(&hadc);

    ADC_ChannelConfTypeDef sConfig = {0};

    // Channel 3 (OPAMP1)
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    // Channel 12 (OPAMP3)
    sConfig.Channel = ADC_CHANNEL_12;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    // Configure DMA (DMA1 Channel 1)
    hdma_adc.Instance = DMA1_Channel1;
    hdma_adc.Init.Request = DMA_REQUEST_ADC1;
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc.Init.Mode = DMA_CIRCULAR;
    hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_adc);

    __HAL_LINKDMA(&hadc, DMA_Handle, hdma_adc);

    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Calibrate ADC
    HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);

    // Start ADC DMA
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)bemf_ring_buffer, BEMF_RING_BUFFER_SIZE);

    // Configure PWM Timer TRGO (Update Event)
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(pwm_timer->getHandle(), &sMasterConfig);

#else
    // --- F4/Standard Setup ---
    ADC_ChannelConfTypeDef sConfig = {0};

    // Configure the ADC peripheral.
    hadc.Instance = (ADC_TypeDef *)pinmap_peripheral(digitalPinToPinName(g_bemf_a_pin), PinMap_ADC);
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; // ADC clock is derived from PCLK2
    hadc.Init.Resolution = ADC_RESOLUTION_12B;          // 12-bit resolution
    hadc.Init.ScanConvMode = ENABLE;                   // Scan multiple channels
    hadc.Init.ContinuousConvMode = ENABLE;             // Continuous conversion
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; // No external trigger
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 2;                     // Two channels to convert
    hadc.Init.DMAContinuousRequests = ENABLE;          // Enable DMA requests
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    HAL_ADC_Init(&hadc);

    // Configure the first ADC channel (BEMF A).
    sConfig.Channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(g_bemf_a_pin), PinMap_ADC));
    sConfig.Rank = 1; // First in the sequence
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    // Configure the second ADC channel (BEMF B).
    sConfig.Channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(g_bemf_b_pin), PinMap_ADC));
    sConfig.Rank = 2; // Second in the sequence
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    // Enable the DMA controller clock.
    __HAL_RCC_DMA2_CLK_ENABLE();

    // Configure the DMA stream for ADC1.
    hdma_adc.Instance = DMA2_Stream0;
    hdma_adc.Init.Channel = DMA_CHANNEL_0; // ADC1 is on DMA2, Channel 0
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; // 16-bit data
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc.Init.Mode = DMA_CIRCULAR; // Continuously fill the buffer
    hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_adc);

    // Link the DMA handle to the ADC handle.
    __HAL_LINKDMA(&hadc, DMA_Handle, hdma_adc);

    // Configure and enable the DMA interrupt.
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    // Start the ADC with DMA.
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)bemf_ring_buffer, BEMF_RING_BUFFER_SIZE);

#if defined(MOTOR_CURRENT_PIN)
    // --- Injected Channel for Current Protection (F4/Standard) ---
    // We use an Injected Channel with Analog Watchdog.
    // Injected channels are scanned independently or via triggers, but crucially they don't
    // mess up the Regular Group DMA buffer layout.

    // Calculate Threshold
    const float V_limit = MAX_CURRENT_AMPS * SHUNT_RESISTOR_OHMS;
    const uint32_t adc_threshold = (uint32_t)((V_limit / 3.3f) * 4095.0f);

    ADC_InjectionConfTypeDef sConfigInjected = {0};
    sConfigInjected.InjectedChannel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(MOTOR_CURRENT_PIN), PinMap_ADC));
    sConfigInjected.InjectedRank = 1;
    sConfigInjected.InjectedNbrOfConversion = 1;
    sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_3CYCLES;
    sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_NONE;
    sConfigInjected.ExternalTrigInjecConv = ADC_INJECTED_SOFTWARE_START;
    // Auto-injection allows injected group to run automatically after regular group
    sConfigInjected.AutoInjectedConv = ENABLE;
    sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
    sConfigInjected.InjectedOffset = 0;
    HAL_ADCEx_InjectedConfigChannel(&hadc, &sConfigInjected);

    // Configure Analog Watchdog on the Injected Channel
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig = {0};
    AnalogWDGConfig.WatchdogNumber = ADC_ANALOGWATCHDOG_1; // F4 usually has 1 watchdog
    AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_INJEC; // Monitor Single Injected Channel
    AnalogWDGConfig.Channel = sConfigInjected.InjectedChannel;
    AnalogWDGConfig.ITMode = ENABLE;
    AnalogWDGConfig.HighThreshold = adc_threshold;
    AnalogWDGConfig.LowThreshold = 0;
    HAL_ADC_AnalogWDGConfig(&hadc, &AnalogWDGConfig);

    // Enable ADC IRQ for Watchdog
    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC_IRQn);

    // Note: AutoInjectedConv = ENABLE means injected conversion starts after regular group.
    // Since regular group is continuous, injected should also run continuously.
#endif

#endif

    pwm_timer->resume();
}

void hal_motor_set_pwm(int duty_cycle, bool forward) {
    uint32_t overflow = pwm_timer->getOverflow();
    uint32_t ticks = map(duty_cycle, 0, 255, 0, overflow);

    if (forward) {
        pwm_timer->setCaptureCompare(pwm_channel_a, ticks, TICK_COMPARE_FORMAT);
        pwm_timer->setCaptureCompare(pwm_channel_b, 0, TICK_COMPARE_FORMAT);
    } else {
        pwm_timer->setCaptureCompare(pwm_channel_a, 0, TICK_COMPARE_FORMAT);
        pwm_timer->setCaptureCompare(pwm_channel_b, ticks, TICK_COMPARE_FORMAT);
    }
}

void hal_motor_deinit() {
    HAL_ADC_Stop_DMA(&hadc);
    HAL_DMA_DeInit(&hdma_adc);
    HAL_ADC_DeInit(&hadc);

#if defined(STM32G4)
    HAL_OPAMP_DeInit(&hopamp1);
    HAL_OPAMP_DeInit(&hopamp3);

#if defined(ENABLE_CURRENT_SENSING)
    HAL_ADC_Stop_DMA(&hadc2);
    HAL_DMA_DeInit(&hdma_adc2);
    HAL_ADC_DeInit(&hadc2);
    HAL_OPAMP_DeInit(&hopamp2);
#endif

#endif

    pwm_timer->pause();
    delete pwm_timer;
}

int hal_motor_get_bemf_buffer(volatile uint16_t** buffer, int* last_write_pos) {
    *buffer = bemf_ring_buffer;
    uint32_t remaining_transfers = __HAL_DMA_GET_COUNTER(&hdma_adc);
    *last_write_pos = BEMF_RING_BUFFER_SIZE - remaining_transfers;
    return BEMF_RING_BUFFER_SIZE;
}

int hal_motor_get_current_buffer(volatile uint16_t** buffer, int* last_write_pos) {
#if defined(STM32G4) && defined(ENABLE_CURRENT_SENSING)
    *buffer = current_ring_buffer;
    uint32_t remaining_transfers = __HAL_DMA_GET_COUNTER(&hdma_adc2);
    *last_write_pos = BEMF_RING_BUFFER_SIZE - remaining_transfers;
    return BEMF_RING_BUFFER_SIZE;
#else
    *buffer = nullptr;
    *last_write_pos = 0;
    return 0;
#endif
}

#endif // ARDUINO_ARCH_STM32
