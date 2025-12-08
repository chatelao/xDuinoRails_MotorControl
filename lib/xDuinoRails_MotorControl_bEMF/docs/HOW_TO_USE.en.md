# How to Use

This guide provides instructions on how to integrate and use the `xDuinoRails_MotorControl_bEMF` library in your project.

## 1. Including the Library

To use the library, you need to include the main header file in your source code:

```cpp
#include <motor_control_hal.h>
```

## 2. Initialization

Before using any other library functions, you must initialize the motor control hardware by calling `hal_motor_init()`. This function sets up the necessary timers, PWM peripherals, ADC, and DMA for your specific hardware.

```cpp
// Example BEMF callback function (see step 4)
void my_bemf_callback(int raw_bemf_value) {
  // Process the BEMF data
}

void setup() {
  // Define the GPIO pins for your hardware configuration
  uint8_t pwm_a_pin = 10;
  uint8_t pwm_b_pin = 11;
  uint8_t bemf_a_pin = A0;
  uint8_t bemf_b_pin = A1;

  // Initialize the motor control HAL
  hal_motor_init(pwm_a_pin, pwm_b_pin, bemf_a_pin, bemf_b_pin, my_bemf_callback);
}
```

## 3. Controlling the Motor

To control the motor's speed and direction, use the `hal_motor_set_pwm()` function. This function should be called from your main loop or a periodic task to update the motor's state.

```cpp
void loop() {
  // Set the motor to 50% duty cycle, forward direction
  hal_motor_set_pwm(128, true);

  delay(1000);

  // Set the motor to 25% duty cycle, reverse direction
  hal_motor_set_pwm(64, false);

  delay(1000);
}
```

## 4. Handling BEMF Data

The library uses a callback function to provide you with real-time BEMF (Back-EMF) data. You need to implement a function with the `hal_bemf_update_callback_t` signature and pass it to `hal_motor_init()`. This function will be called from an interrupt context whenever a new BEMF measurement is available.

**Important:** Keep the code within the callback function as short and efficient as possible, as it runs in an interrupt context. Avoid lengthy operations or blocking calls.

```cpp
void my_bemf_callback(int raw_bemf_value) {
  // Example: Store the latest BEMF value in a volatile variable
  // for processing in the main loop.
  volatile int latest_bemf = raw_bemf_value;
}
```

## 5. Diagnostics and Debugging

For debugging and visualization purposes, you can get direct access to the BEMF ADC sample buffer using `hal_motor_get_bemf_buffer()`. This is an advanced feature and is not typically needed for normal operation.

```cpp
void debug_bemf_buffer() {
  volatile uint16_t* bemf_buffer;
  int last_write_pos;
  int buffer_size = hal_motor_get_bemf_buffer(&bemf_buffer, &last_write_pos);

  // You can now inspect the contents of bemf_buffer
}
```

## 6. Platform Specifics

This is a Hardware Abstraction Layer (HAL), and the underlying implementation is specific to the microcontroller you are using. Ensure that you have the correct hardware definitions and PlatformIO environment selected for your target board.
