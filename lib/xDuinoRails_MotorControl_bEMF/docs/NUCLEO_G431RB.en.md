# Nucleo G431RB Implementation

This document describes the implementation details for the STM32 Nucleo G431RB board within the `xDuinoRails_MotorControl_bEMF` library.

## Hardware Overview

The Nucleo G431RB features the STM32G431RB microcontroller, which is specifically designed for mixed-signal applications like motor control. It includes advanced analog peripherals such as OpAmps, Comparators, and fast ADCs.

## Implementation Details

### OpAmp Integration

To ensure precise BEMF (Back Electromotive Force) measurement, this library utilizes the internal Operational Amplifiers (OpAmps) of the STM32G431RB.
The OpAmps are configured in **Follower Mode** (Unity Gain Buffer). This provides a high-impedance input for the motor terminals, preventing the ADC sampling process from affecting the voltage reading and protecting the internal ADC circuitry.

*   **OPAMP1:** Buffers the signal from **PA1** (Arduino **A1**).
*   **OPAMP3:** Buffers the signal from **PB0** (Arduino **A3**).

### ADC Configuration

*   **ADC1** is used to sample the outputs of OPAMP1 and OPAMP3.
*   The sampling is triggered by **TIM1** (Timer 1) via the TRGO (Trigger Output) signal.
*   **DMA** (Direct Memory Access) is used to transfer the conversion results to a ring buffer in memory, ensuring zero CPU overhead for data acquisition.

### PWM Generation

*   **TIM1** is used to generate the PWM signals for motor control.
*   The timer update event synchronizes the ADC sampling, ensuring BEMF is measured at the appropriate time in the PWM cycle (typically during the OFF phase).

## Pin Mapping

| Function | Pin Name | Arduino Pin | Description |
| :--- | :--- | :--- | :--- |
| **PWM A** | PA8 | D7 | Timer 1 Channel 1 Output |
| **PWM B** | PA9 | D8 | Timer 1 Channel 2 Output |
| **BEMF A** | PA1 | A1 | OPAMP1 Non-Inverting Input |
| **BEMF B** | PB0 | A3 | OPAMP3 Non-Inverting Input |

> **Note:** Ensure your motor driver logic matches this pinout. The OpAmp inputs should be connected to the motor terminals (or a voltage divider if the voltage exceeds 3.3V). Since the Nucleo board is 3.3V logic, ensure the BEMF voltage does not exceed 3.3V.

### Current Sensing (Optional)

The library supports optional Current Sensing using **OPAMP2** and **ADC2**.
This feature is enabled by the build flag `-D ENABLE_CURRENT_SENSING`.

*   **Input Pin:** PA7 (Arduino D11).
*   **OPAMP2:** Configured in Follower Mode to buffer the shunt voltage.
*   **ADC:** ADC2 samples the buffered signal synchronously with PWM.

## Usage

To use this board, select the `nucleo_g431rb` environment in PlatformIO.

```ini
[env:nucleo_g431rb]
platform = ststm32
board = nucleo_g431rb
framework = arduino
lib_deps = ...

; For Current Sensing
[env:nucleo_g431rb_shunt]
extends = env:nucleo_g431rb
build_flags = -D ENABLE_CURRENT_SENSING
```
