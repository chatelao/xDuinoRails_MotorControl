# FiftyPercent Example

This example provides a minimal demonstration of the motor control library. It initializes the motor and runs it at a constant 50% speed in the forward direction.

## How it Works

The `setup()` function initializes the serial communication and the motor control hardware abstraction layer (`hal_motor_init`). It then calls `hal_motor_set_pwm` to set the motor's speed to 127 (which corresponds to roughly 50% of the maximum PWM duty cycle of 255).

The `loop()` function is empty, as the motor speed is set only once.

## Hardware Setup

The hardware setup and pin connections are identical to the other examples, such as `RotaryEncoderWithBEMF`. Please refer to the `README.md` in that directory for detailed wiring diagrams.
