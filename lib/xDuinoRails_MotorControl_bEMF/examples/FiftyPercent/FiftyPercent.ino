/**
 * @file FiftyPercent.ino
 * @brief A minimalistic example to run the motor at 50% speed.
 *
 * This sketch demonstrates the simplest way to run the motor. It initializes
 * the hardware abstraction layer and sets the motor's PWM duty cycle to 50%
 * of its maximum value.
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

void setup() {
  // Start serial communication for debugging output.
  Serial.begin(115200);
  Serial.println("Fifty Percent Motor Example");

  // Initialize the motor hardware abstraction layer.
  // We provide nullptr for the BEMF callback as we don't need it.
  hal_motor_init(MOTOR_PWM_A_PIN, MOTOR_PWM_B_PIN, MOTOR_BEMF_A_PIN, MOTOR_BEMF_B_PIN, nullptr);

  // Set motor speed to 50% (127 out of 255) in the forward direction.
  const int fifty_percent_speed =  127;
  const bool forward_direction  = true;
  hal_motor_set_pwm(fifty_percent_speed, forward_direction);

#ifdef LED_EDITION
  // Initialize Neopixel for LED_EDITION
  pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
  digitalWrite(NEOPIXEL_POWER_PIN, HIGH);
  delay(10); // Wait for power to stabilize
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green to indicate 50% speed
  pixels.show();
#endif

  Serial.println("Motor running at 50% speed.");
}

void loop() {
  // Nothing to do here. The motor speed is set once in setup().
}
