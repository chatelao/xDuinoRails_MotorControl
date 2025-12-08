/**
 * @file SineWaveSpeed.ino
 * @brief An example of varying motor speed in a sine wave pattern.
 *
 * This sketch demonstrates how to control the motor's speed to follow a sine
 * wave, varying between 0% and 75% of its maximum speed over a period of 2.5
 * seconds.
 *
 * ## How it Works
 * - The motor speed is modulated using a sine function.
 * - The sine wave is scaled to have a minimum of 0% and a maximum of 75% duty
 *   cycle.
 * - One full cycle of the sine wave (from 0% up to 75% and back to 0%) is
 *   completed every 2.5 seconds.
 */

#include <Arduino.h>
#include "motor_control_hal.h"
#include "StatusLED.h"

#if defined(ARDUINO_SEEED_XIAO_RP2040)
#include <Adafruit_NeoPixel.h>
#endif

// --- Pin Definitions ---
#if defined(ARDUINO_SEEED_XIAO_RP2040)
    // --- Neopixel LED ---
    #define NEOPIXEL_PIN 12
    #define NEOPIXEL_POWER_PIN 11
    Adafruit_NeoPixel pixels(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

    #ifdef LED_EDITION
        // For Seeed XIAO RP2040 "LED Edition"
        const int MOTOR_PWM_A_PIN       = 17; // Red LED
        const int MOTOR_PWM_B_PIN       = 16; // Green LED
        const int MOTOR_BEMF_A_PIN      = D7;
        const int MOTOR_BEMF_B_PIN      = D8;
    #else
        // For standard Seeed XIAO RP2040
        const int MOTOR_PWM_A_PIN       =  D9;
        const int MOTOR_PWM_B_PIN       = D10;
        const int MOTOR_BEMF_A_PIN      =  D7;
        const int MOTOR_BEMF_B_PIN      =  D8;
    #endif
#else
// Default pins for other boards
const int MOTOR_PWM_A_PIN       =   7;
const int MOTOR_PWM_B_PIN       =   8;
const int MOTOR_BEMF_A_PIN      =  A3;
const int MOTOR_BEMF_B_PIN      =  A2;
#endif

const int STATUS_LED_PIN        = LED_BUILTIN;

// --- Status LED Instance ---
StatusLED status_led(STATUS_LED_PIN);

// --- Sine Wave Parameters ---
const float SINE_WAVE_PERIOD    = 2500; // 2.5 seconds in milliseconds
const int MIN_PWM_DUTY_CYCLE    =    0;   // 0% of 255
const int MAX_PWM_DUTY_CYCLE    =  191;  // 75% of 255
bool motorDirection             = true;   // Motor direction: true for forward

void setup() {
  Serial.begin(115200);
  Serial.println("Sine Wave Motor Speed Control Example");

  // Initialize the motor hardware abstraction layer.
  hal_motor_init(MOTOR_PWM_A_PIN, MOTOR_PWM_B_PIN, MOTOR_BEMF_A_PIN, MOTOR_BEMF_B_PIN, NULL);

  // Initialize the status LED.
  status_led.begin();
  status_led.on();

#if defined(ARDUINO_SEEED_XIAO_RP2040)
  // Initialize Neopixel
  pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
  digitalWrite(NEOPIXEL_POWER_PIN, HIGH);
  delay(10); // Wait for power to stabilize
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue to indicate SineWave mode
  pixels.show();
#endif
}

void loop() {
  // Calculate the elapsed time.
  float elapsedTime = millis();

  // Calculate the sine wave value.
  float sineValue = sin(2 * PI * (elapsedTime / SINE_WAVE_PERIOD));

  // Map the sine value (-1 to 1) to the PWM range (MIN_PWM to MAX_PWM).
  int pwmValue = map(sineValue * 1000, -1000, 1000, MIN_PWM_DUTY_CYCLE, MAX_PWM_DUTY_CYCLE);

  // Set the motor PWM.
  hal_motor_set_pwm(pwmValue, motorDirection);

#if defined(ARDUINO_SEEED_XIAO_RP2040)
  // Indicate motor direction with color (Green for Forward, Red for Backward)
  if (motorDirection) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  } else {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  }
  pixels.show();
#endif

  // Update the status LED.
  status_led.update();

  // Print the current PWM value for debugging.
  Serial.print("PWM: ");
  Serial.println(pwmValue);

  // A small delay to prevent spamming the serial port.
  delay(10);
}
