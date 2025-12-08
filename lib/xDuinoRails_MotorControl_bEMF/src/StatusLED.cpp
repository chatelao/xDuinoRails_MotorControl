#include "StatusLED.h"

StatusLED::StatusLED(uint8_t pin) : _pin(pin), _is_on(false), _blink_start_time(0), _blink_duration(0) {}

void StatusLED::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void StatusLED::on() {
    digitalWrite(_pin, HIGH);
    _is_on = true;
    _blink_duration = 0; // Cancel any ongoing blink
}

void StatusLED::off() {
    digitalWrite(_pin, LOW);
    _is_on = false;
    _blink_duration = 0; // Cancel any ongoing blink
}

void StatusLED::blink(unsigned long duration_ms) {
    on();
    _blink_start_time = millis();
    _blink_duration = duration_ms;
}

void StatusLED::update() {
    if (_blink_duration > 0 && (millis() - _blink_start_time) >= _blink_duration) {
        off();
        _blink_duration = 0; // Reset blink
    }
}
