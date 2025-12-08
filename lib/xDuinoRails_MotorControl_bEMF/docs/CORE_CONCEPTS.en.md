# Core Concepts

This document explains the core concepts and the architecture of the `xDuinoRails_MotorControl_bEMF` library.

## 1. Hardware Abstraction Layer (HAL)

The `xDuinoRails_MotorControl_bEMF` library is designed as a Hardware Abstraction Layer (HAL). This means that it provides a standardized, high-level API for motor control, while hiding the low-level, platform-specific details of the hardware implementation.

The main benefits of this approach are:
- **Portability:** Your application code is not tied to a specific microcontroller. As long as a HAL implementation exists for your target platform, your code should work with minimal changes.
- **Simplicity:** The HAL provides a simplified and consistent interface to complex hardware peripherals like timers, PWM generators, ADCs, and DMA controllers.
- **Maintainability:** The platform-specific code is neatly separated from the application logic, making both easier to maintain and debug.

The public API of the HAL is defined in `motor_control_hal.h`. The implementations for the supported microcontrollers (e.g., ESP32, RP2040, STM32) are provided in separate `.cpp` files.

## 2. PWM Motor Control

Pulse-Width Modulation (PWM) is a technique for controlling the amount of power sent to a device, in this case, a DC motor. By varying the duty cycle of a square wave signal, we can effectively control the motor's speed.

The library uses the microcontroller's hardware PWM peripherals to generate these signals efficiently, without consuming CPU resources. The `hal_motor_set_pwm()` function allows you to set both the duty cycle and the direction of the motor.

## 3. Back-EMF (BEMF) Measurement

When a DC motor spins, it also acts as a generator, producing a voltage known as Back-EMF (BEMF). The magnitude of this BEMF is directly proportional to the motor's rotational speed. By measuring the BEMF, we can infer the motor's speed without needing a separate sensor, enabling what is known as "sensorless" motor control.

To measure the BEMF, the library uses the microcontroller's Analog-to-Digital Converter (ADC). Two ADC channels are used to measure the voltage at the two motor terminals. The difference between these two readings gives a differential BEMF measurement, which is robust to common-mode noise.

## 4. DMA-based ADC Sampling

To achieve high-performance, non-blocking BEMF measurement, the library leverages the microcontroller's Direct Memory Access (DMA) controller.

The DMA is configured to automatically transfer the ADC conversion results into a circular buffer in memory. This happens in the background, without any CPU intervention. Once a new BEMF sample is available in the buffer, the HAL triggers an interrupt and calls the user-provided callback function.

This architecture ensures that the BEMF data is sampled at a consistent rate, and that the main application is not blocked while waiting for ADC conversions. It is a highly efficient and scalable approach for real-time motor control.
