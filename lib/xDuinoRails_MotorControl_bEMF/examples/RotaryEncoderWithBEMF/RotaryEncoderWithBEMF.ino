/**
 * @file RotaryEncoderWithBEMF.ino
 * @brief An example of controlling motor speed with a rotary encoder and reading BEMF.
 *
 * This sketch demonstrates how to use a standard KY-040 rotary encoder to
 * control the motor's speed and its integrated push-button to stop the motor or
 * change its direction. It also shows how to read the motor's speed using BEMF.
 *
 * ## How it Works
 * - Turning the encoder knob increases or decreases the motor's target speed.
 *   One full rotation of a 24-detent encoder will ramp the speed from 0 to 100%.
 * - Pressing the encoder's push-button has two functions:
 *   1. If the motor is currently moving, it acts as an emergency stop, setting
 *      the target speed to 0.
 *   2. If the motor is stopped, it toggles the direction of travel for the next
 *      time the motor starts (Forward -> Reverse -> Forward).
 *
 * ## Hardware Setup
 * For detailed wiring instructions, please see the README.md file located in
 * this same directory.
 */

#include <Arduino.h>
#include "motor_control_hal.h"
#include <RotaryEncoder.h>
#include "StatusLED.h"

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

// Define the pins for the rotary encoder.
const int ENCODER_PIN_A         =   0;      // CLK pin
const int ENCODER_PIN_B         =   1;      // DT pin
const int ENCODER_SWITCH_PIN    =   9; // SW pin

// Define the pin for the status LED.
const int STATUS_LED_PIN        = LED_BUILTIN;
const int BEMF_STATUS_LED_PIN   =   2;

// --- Status LED Instance ---
StatusLED status_led(STATUS_LED_PIN);
StatusLED bemf_status_led(BEMF_STATUS_LED_PIN);

#ifdef LED_EDITION
// --- Neopixel LED for LED_EDITION ---
#define NEOPIXEL_PIN 12
#define NEOPIXEL_POWER_PIN 11
Adafruit_NeoPixel pixels(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

// --- BEMF Callback ---
// This function is called from an interrupt whenever a new BEMF value is available.
void on_bemf_update(int raw_bemf) {
  // For this simple example, we'll just print the raw value.
  // In a real application, you would filter and process this data.
  bemf_status_led.blink(50);
  Serial.print("Raw BEMF: ");
  Serial.println(raw_bemf);
}

// --- Encoder Instance ---
// Create an instance of the rotary encoder.
RotaryEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// --- Control Logic Parameters ---
const long ENCODER_MIN_POSITION =   0;
const long ENCODER_MAX_POSITION =  24; // Assumes a standard 24-detent encoder for one full turn.
const int MAX_PWM_DUTY_CYCLE    = 255;   // The PWM duty cycle at 100% encoder turn.
bool motorDirection             = true;           // Current motor direction: true for forward, false for reverse.
int current_speed               =   0;                // Current motor speed

// --- Button Debouncing ---
// Variables to handle button debouncing to prevent multiple triggers from a single press.
unsigned long lastButtonPressTime   =  0;
const unsigned long DEBOUNCE_DELAY      = 50; // 50 milliseconds

void setup() {
  // Start serial communication for debugging output.
  Serial.begin(115200);
  Serial.println("Rotary Encoder Motor Control with HAL Example");
  Serial.println("Turn the knob to change speed, press it to stop or change direction.");

  // Initialize the motor hardware abstraction layer.
  hal_motor_init(MOTOR_PWM_A_PIN, MOTOR_PWM_B_PIN, MOTOR_BEMF_A_PIN, MOTOR_BEMF_B_PIN, on_bemf_update);

  // Set up the encoder's switch pin with an internal pull-up resistor.
  // This means the pin will be HIGH by default and LOW when the button is pressed.
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);

  // Start the encoder at the minimum position.
  encoder.setPosition(ENCODER_MIN_POSITION);

  // Initialize the status LED.
  status_led.begin();
  bemf_status_led.begin();

#ifdef LED_EDITION
  // Initialize Neopixel for LED_EDITION
  pinMode(NEOPIXEL_POWER_PIN, OUTPUT);
  digitalWrite(NEOPIXEL_POWER_PIN, HIGH);
  delay(10); // Wait for power to stabilize
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red for stop
  pixels.show();
#endif
}

void loop() {
  // Poll the encoder for any new movement.
  encoder.tick();

  // Update the status LED.
  status_led.update();
  bemf_status_led.update();

  // --- Encoder Logic for Speed Control ---
  long newPosition = encoder.getPosition();

  // Constrain the encoder's value to stay within our defined min/max range.
  if (newPosition < ENCODER_MIN_POSITION) {
    newPosition = ENCODER_MIN_POSITION;
    encoder.setPosition(newPosition);
  } else if (newPosition > ENCODER_MAX_POSITION) {
    newPosition = ENCODER_MAX_POSITION;
    encoder.setPosition(newPosition);
  }

  // Map the constrained encoder position to the PWM duty cycle.
  int newSpeed = map(newPosition, ENCODER_MIN_POSITION, ENCODER_MAX_POSITION, 0, MAX_PWM_DUTY_CYCLE);
  if (newSpeed != current_speed) {
    hal_motor_set_pwm(newSpeed, motorDirection);
    current_speed = newSpeed;
    if (newSpeed > 0) {
      status_led.on();
#ifdef LED_EDITION
      if (motorDirection) {
        pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // Green for forward
      } else {
        pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue for backward
      }
      pixels.show();
#endif
    } else {
      status_led.off();
#ifdef LED_EDITION
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red for stop
      pixels.show();
#endif
    }
    Serial.print("New Speed (PWM): ");
    Serial.println(newSpeed);
  }

  // --- Button Logic for Stop/Direction Control ---
  // Check if the button is pressed (pin is LOW) and if enough time has passed since the last press.
  if (digitalRead(ENCODER_SWITCH_PIN) == LOW && (millis() - lastButtonPressTime) > DEBOUNCE_DELAY) {
    if (current_speed > 0) {
      // If the motor is currently moving, stop it and reset the encoder position.
      hal_motor_set_pwm(0, motorDirection);
      current_speed = 0;
      encoder.setPosition(ENCODER_MIN_POSITION);
      status_led.off();
#ifdef LED_EDITION
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red for stop
      pixels.show();
#endif
      Serial.println("Motor stopped.");
    } else {
      // If the motor is stopped, toggle the direction for the next run.
      motorDirection = !motorDirection;
      status_led.blink(500); // Blink for 500ms to indicate direction change
      Serial.print("Direction changed to: ");
      Serial.println(motorDirection ? "Forward" : "Reverse");
    }
    // Record the time of this press to handle debouncing.
    lastButtonPressTime = millis();
  }
}
