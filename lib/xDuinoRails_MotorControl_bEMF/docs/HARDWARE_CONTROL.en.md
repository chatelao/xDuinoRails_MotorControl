# Hardware-Accelerated Motor Control (CPU-Assisted)

This document describes the hardware-accelerated motor control and BEMF measurement implemented for the RP2040. This implementation uses standard RP2040 peripherals (PWM, ADC, DMA) and minimal CPU support to achieve precise and CPU-efficient control.

## Architecture Overview

The system is based on a chain of hardware blocks synchronized by short CPU interrupts:

1.  **PWM Slice (Motor Control):** Generates the 25 kHz PWM signal driving the motor via the BDR6133 driver. At the end of each PWM cycle, this block triggers a hardware interrupt (`PWM_IRQ_WRAP`).
2.  **CPU (PWM ISR):** The extremely short `on_pwm_wrap()` ISR is called. Its sole task is to start a high-precision hardware timer for the BEMF delay.
3.  **Hardware Timer:** Runs for the exact delay time of 10µs and triggers a callback function upon expiration.
4.  **CPU (Timer Callback):** The `delayed_adc_trigger_callback` function is called. Its sole task is to start the ADC measurement cycle.
5.  **ADC (Analog-to-Digital Converter):** Performs a "Round-Robin" measurement on the two BEMF pins.
6.  **DMA (Direct Memory Access):** Automatically copies the ADC results into a ring buffer in RAM without further burdening the CPU.

## The Process in Detail

### 1. PWM Phase (Motor Driven)

- The `loop()` function calls `update_pwm_duty_cycle()` to set the PWM levels for the H-bridge pins.
- **Forward:** Pin INA receives the PWM signal, Pin INB is held LOW.
- **Reverse:** Pin INB receives the PWM signal, Pin INA is held LOW.
- The PWM hardware ensures that during the OFF phase both pins are LOW, placing the BDR6133 into high-impedance "Stand-by" mode.

### 2. BEMF Measurement Phase (CPU-Assisted Trigger)

- **PWM Wrap Interrupt:** The end of the PWM cycle triggers the `PWM_IRQ_WRAP` interrupt. The ISR `on_pwm_wrap()` is executed immediately.
- **Stabilization Delay:** The ISR starts a one-shot hardware timer (`add_alarm_in_us`) with a precise delay of 10µs.
- **ADC Trigger:** After 10µs elapses, the timer callback function `delayed_adc_trigger_callback` is called, which simply starts the ADC using `adc_run(true)`.

### 3. Data Processing Phase (DMA-Controlled)

- **Automatic Measurement & DMA Transfer:** The ADC performs a measurement on both BEMF pins. After each measurement, the ADC sends a DREQ signal to the DMA controller, which immediately copies the value into the `bemf_ring_buffer`.
- **DMA Interrupt & Data Processing:** Once the DMA controller has filled the buffer, it triggers an interrupt. The ISR `dma_irq_handler()` is called and performs the average calculation, filtering, pulse detection, and PI control.

This cycle repeats continuously at 25 kHz. The CPU is only required for two extremely short, non-time-critical ISR calls per cycle, while precise timing is handled entirely by hardware.
