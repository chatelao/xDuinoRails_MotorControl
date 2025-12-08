# Developer Reference

This document provides a detailed reference for the API of the `xDuinoRails_MotorControl_bEMF` library.

## Typedefs

### `hal_bemf_update_callback_t`

```cpp
typedef void (*hal_bemf_update_callback_t)(int raw_bemf_value);
```

A function pointer type for the BEMF update callback.

- **Parameters:**
  - `raw_bemf_value`: The raw, unfiltered differential BEMF value.
- **Context:** This function is called from an interrupt service routine (ISR). It should be kept as short and efficient as possible.

---

## Functions

### `hal_motor_init`

```cpp
void hal_motor_init(
  uint8_t pwm_a_pin,
  uint8_t pwm_b_pin,
  uint8_t bemf_a_pin,
  uint8_t bemf_b_pin,
  hal_bemf_update_callback_t callback
);
```

Initializes the low-level hardware for motor control. This function must be called once before any other function in this library.

- **Parameters:**
  - `pwm_a_pin`: The GPIO pin for PWM channel A.
  - `pwm_b_pin`: The GPIO pin for PWM channel B.
  - `bemf_a_pin`: The ADC pin for BEMF measurement A.
  - `bemf_b_pin`: The ADC pin for BEMF measurement B.
  - `callback`: A pointer to the function to be called when new BEMF data is available.

---

### `hal_motor_set_pwm`

```cpp
void hal_motor_set_pwm(int duty_cycle, bool forward);
```

Sets the motor's PWM duty cycle and direction.

- **Parameters:**
  - `duty_cycle`: The PWM duty cycle, typically from 0 to 255. The exact range may vary depending on the hardware's PWM resolution.
  - `forward`: The motor direction. `true` for forward, `false` for reverse.

---

### `hal_motor_get_bemf_buffer`

```cpp
int hal_motor_get_bemf_buffer(
  volatile uint16_t** buffer,
  int* last_write_pos
);
```

Retrieves the internal BEMF ring buffer for diagnostics and debugging.

- **Parameters:**
  - `buffer` (out): A pointer to a `uint16_t` pointer. This will be updated to point to the internal ring buffer.
  - `last_write_pos` (out): A pointer to an integer. This will be updated with the index of the last written sample in the buffer.
- **Returns:**
  - The total size of the ring buffer.
