# Low-Level HAL Motor Control with Rotary Encoder

This example demonstrates how to use the Hardware Abstraction Layer (HAL) for direct, low-level motor control. It shows how to control the motor's PWM duty cycle with a rotary encoder and receive raw BEMF data through a callback.

## How it Works

- **PWM Control:** Turning the encoder directly adjusts the PWM duty cycle sent to the motor, ranging from 0 to 255.
- **Stop & Direction:** Pressing the encoder's button will stop the motor or toggle its direction.
- **BEMF Callback:** BEMF data is received via an interrupt-driven callback and printed to the serial port. This example does not perform any filtering or processing on the raw BEMF values.

## Hardware Setup

Connect the rotary encoder to the XIAO RP2040 as follows:

| Encoder Pin | XIAO RP2040 Pin |
| :---------- | :-------------- |
| CLK         | D0              |
| DT          | D1              |
| SW (Switch) | D9              |
| + (VCC)     | 3.3V            |
| GND         | GND             |

**Note:** The example uses the microcontroller's internal pull-up resistors for the CLK, DT, and SW pins. Therefore, you do not need to add external pull-up resistors to your circuit.
