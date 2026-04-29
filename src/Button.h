#pragma once

#include <Arduino.h>

class Button
{
private:
    uint8_t pin;
    bool lastReading = HIGH;
    bool stableState = HIGH;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 40;

public:
    Button(uint8_t pin) : pin(pin) {}

    void begin()
    {
        pinMode(pin, INPUT_PULLUP);
    }

    bool wasPressed()
    {
        bool reading = digitalRead(pin);

        if (reading != lastReading)
        {
            lastDebounceTime = millis();
        }

        lastReading = reading;

        if ((millis() - lastDebounceTime) > debounceDelay)
        {
            if (reading != stableState)
            {
                stableState = reading;

                if (stableState == LOW)
                {
                    return true;
                }
            }
        }

        return false;
    }
};