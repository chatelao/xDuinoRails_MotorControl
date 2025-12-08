/**
 * @file MotorSerialDebug.ino
 * @brief A serial control example for motor speed and direction.
 *
 * This sketch allows you to control the motor speed and direction using
 * the Serial monitor.
 *
 * Controls:
 * - '0' to '9': Set speed from 0% to 90%.
 * - '-': Toggle direction.
 *
 * ## Hardware Setup
 * For detailed wiring instructions, please see the README.md file located in
 * this same directory.
 */

#include <Arduino.h>
#include "motor_control_hal.h"

#ifdef LED_EDITION
#include <Adafruit_NeoPixel.h>
#endif

// --- Pin Definitions ---
#if defined(ARDUINO_SEEED_XIAO_RP2040)
    #ifdef LED_EDITION
        // For Seeed XIAO RP2040 "LED Edition"
        const int MOTOR_PWM_A_PIN       = 17; // Red LED
        const int MOTOR_PWM_B_PIN       = 16; // Green LED
        const int MOTOR_BEMF_A_PIN      = D7;
        const int MOTOR_BEMF_B_PIN      = D8;

        // --- Neopixel LED for LED_EDITION ---
        #define NEOPIXEL_PIN 12
        #define NEOPIXEL_POWER_PIN 11
        Adafruit_NeoPixel pixels(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
    #else
        // For standard Seeed XIAO RP2040
        const int MOTOR_PWM_A_PIN       =  D9;
        const int MOTOR_PWM_B_PIN       = D10;
        const int MOTOR_BEMF_A_PIN      =  D7;
        const int MOTOR_BEMF_B_PIN      =  D8;
    #endif
#elif defined(ARDUINO_NUCLEO_G431RB)
// Pins for Nucleo G431RB (Using internal OpAmps)
const int MOTOR_PWM_A_PIN       =   D7;
const int MOTOR_PWM_B_PIN       =   D8;
const int MOTOR_BEMF_A_PIN      =  A1; // PA1 -> OPAMP1_VINP
const int MOTOR_BEMF_B_PIN      =  A3; // PB0 -> OPAMP3_VINP

#else
// Default pins for other boards
const int MOTOR_PWM_A_PIN       =   7;
const int MOTOR_PWM_B_PIN       =   8;
const int MOTOR_BEMF_A_PIN      =  A3;
const int MOTOR_BEMF_B_PIN      =  A2;
#endif

int current_speed = 0;
bool current_direction = true;

void setup() {
  // Start serial communication for debugging output.
  Serial.begin(115200);
  Serial.println("MotorSerialDebug Example");
  Serial.println("Send 0-9 for speed (0% - 90%), '-' to toggle direction");

  // Initialize the motor hardware abstraction layer.
  // We provide nullptr for the BEMF callback as we don't need it.
  hal_motor_init(MOTOR_PWM_A_PIN, MOTOR_PWM_B_PIN, MOTOR_BEMF_A_PIN, MOTOR_BEMF_B_PIN, nullptr);

  // Initial state: Stopped, Forward
  hal_motor_set_pwm(0, true);

#ifdef LED_EDITION
  // Initialize Neopixel for LED_EDITION
  pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
  digitalWrite(NEOPIXEL_POWER_PIN, HIGH);
  delay(10); // Wait for power to stabilize
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
  pixels.show();
#endif
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();

    if (c >= '0' && c <= '9') {
      int digit = c - '0';
      // Map 0-9 to 0-90% speed (approx 0-230 PWM)
      // 100% speed is 255.
      // digit * 10 is percentage.
      // pwm = (digit * 10 * 255) / 100 = (digit * 255) / 10
      current_speed = (digit * 255) / 10;

      hal_motor_set_pwm(current_speed, current_direction);
      Serial.print("Set Speed: ");
      Serial.print(digit * 10);
      Serial.println("%");

    } else if (c == '-') {
      current_direction = !current_direction;
      hal_motor_set_pwm(current_speed, current_direction);
      Serial.print("Set Direction: ");
      Serial.println(current_direction ? "Forward" : "Reverse");
    }

    #ifdef LED_EDITION
    if (current_speed > 0) {
        if (current_direction) {
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green for Forward
        } else {
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red for Reverse
        }
    } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue for Stopped
    }
    pixels.show();
    #endif
  }
}
