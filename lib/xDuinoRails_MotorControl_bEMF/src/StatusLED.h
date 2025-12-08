#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

class StatusLED {
public:
    StatusLED(uint8_t pin);
    void begin();
    void on();
    void off();
    void blink(unsigned long duration_ms);
    void update();

private:
    uint8_t _pin;
    bool _is_on;
    unsigned long _blink_start_time;
    unsigned long _blink_duration;
};

#endif // STATUS_LED_H
