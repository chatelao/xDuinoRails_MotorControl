# Sine Wave Speed Control Example

This example demonstrates how to control a motor's speed to follow a sine wave pattern, varying between 25% and 75% of its maximum speed over a period of 2.5 seconds.

## How it Works

The motor speed is modulated using a sine function. The `loop()` function continuously calculates the elapsed time and uses it to generate a sine wave. This sine value, which oscillates between -1 and 1, is then mapped to a PWM duty cycle range of 25% to 75%. This results in the motor speed smoothly accelerating and decelerating in a wave-like pattern.

## Hardware Setup

The hardware setup for this example is minimal. You only need to connect the motor to the appropriate PWM and BEMF pins as defined in the sketch. For the Seeed XIAO RP2040, the pins are:

- **MOTOR_PWM_A_PIN:** D9
- **MOTOR_PWM_B_PIN:** D10
- **MOTOR_BEMF_A_PIN:** D7
- **MOTOR_BEMF_B_PIN:** D8

For other boards, the default pins are:

- **MOTOR_PWM_A_PIN:** 7
- **MOTOR_PWM_B_PIN:** 8
- **MOTOR_BEMF_A_PIN:** A3
- **MOTOR_BEMF_B_PIN:** A2

## LED Configuration

### Seeed XIAO RP2040

The example utilizes the onboard Neopixel LED on the Seeed XIAO RP2040 to indicate the motor status:

- **Startup:** Blue
- **Forward:** Green
- **Reverse:** Red

### LED Edition (Seeed XIAO RP2040)

For testing purposes without a physical motor, the `LED_EDITION` build flag can be used (e.g., `pio run -e seeed_xiao_rp2040_led`). This configuration reassigns the motor PWM pins to the onboard user LEDs:

- **MOTOR_PWM_A_PIN:** 17 (Red LED)
- **MOTOR_PWM_B_PIN:** 16 (Green LED)

The firmware automatically handles the active-low logic of these LEDs, allowing them to visually represent the motor's speed and direction (fading in and out based on the sine wave).
