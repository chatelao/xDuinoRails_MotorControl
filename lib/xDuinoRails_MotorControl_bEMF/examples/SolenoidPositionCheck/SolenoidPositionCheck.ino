/**
 * @file SolenoidPositionCheck.ino
 * @brief Example for checking solenoid position using ping response.
 *
 * This sketch demonstrates how to use the `hal_motor_check_solenoid_position`
 * function to diagnose the position of a solenoid or motor by measuring the
 * inductive kickback (BEMF) response to a short pulse.
 */

#include <Arduino.h>
#include "motor_control_hal.h"

// --- Pin Definitions ---
#if defined(ARDUINO_SEEED_XIAO_RP2040)
    // For Seeed XIAO RP2040
    const int MOTOR_PWM_A_PIN       =  D9;
    const int MOTOR_PWM_B_PIN       = D10;
    const int MOTOR_BEMF_A_PIN      =  D7;
    const int MOTOR_BEMF_B_PIN      =  D8;
#elif defined(ARDUINO_NUCLEO_G431RB)
    // Pins for Nucleo G431RB
    const int MOTOR_PWM_A_PIN       =   D7;
    const int MOTOR_PWM_B_PIN       =   D8;
    const int MOTOR_BEMF_A_PIN      =  A1;
    const int MOTOR_BEMF_B_PIN      =  A3;
#else
    // Default pins
    const int MOTOR_PWM_A_PIN       =   7;
    const int MOTOR_PWM_B_PIN       =   8;
    const int MOTOR_BEMF_A_PIN      =  A3;
    const int MOTOR_BEMF_B_PIN      =  A2;
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Solenoid Position Check Example");

  // Initialize the motor HAL. Callback is not needed for this synchronous test.
  hal_motor_init(MOTOR_PWM_A_PIN, MOTOR_PWM_B_PIN, MOTOR_BEMF_A_PIN, MOTOR_BEMF_B_PIN, nullptr);
}

void loop() {
  Serial.println("Checking position...");

  int response_a = 0;
  int response_b = 0;

  // Parameters:
  // PWM: 200 (out of 255) - Strong enough to create field but short enough not to move much?
  // Duration: 10 ms - Short ping
  // Delay: 100 us - Measurement delay
  hal_motor_check_solenoid_position(200, 10, 100, &response_a, &response_b);

  Serial.print("Response A: ");
  Serial.print(response_a);
  Serial.print(" | Response B: ");
  Serial.println(response_b);

  if (response_a > response_b) {
      Serial.println("Position: Likely A (or closer to A side)");
  } else if (response_b > response_a) {
      Serial.println("Position: Likely B (or closer to B side)");
  } else {
      Serial.println("Position: Indeterminate");
  }

  delay(2000);
}
