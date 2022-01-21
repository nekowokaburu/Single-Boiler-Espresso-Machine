#ifndef __LED_HPP
#define __LED_HPP

#include "settings.hpp"

class LED
{
public:
    // A byte where every bit sets the led on (1) or off (0) and is called periodicially.
    // The naming relates to the musical notation and horses.
    enum class Signal : uint8_t
    {
        Off = 0x0,           // 0000 0000
        Whole = 0xFF,        // 1111 1111 Whole
        Half = 0xF0,         // 1111 0000 Half
        Quarter = 0xC0,      // 1100 0000 Quarter
        Eights = 0x80,       // 1000 0000 Eights
        Sixteenth = 0x88,    // 1000 1000 Sixteenth
        ThirtySecond = 0xAA, // 1010 1010 ThirtySecond
        Trab = 0xA0,         // 1010 0000 Trab
        Gallop = 0xA8        // 1010 1000 Gallop
    };

    LED()
        : lastTime_{0}, signalStyle_{Signal::Off}
    {
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);
    }

    // Update in loop to blink the led in the desired way
    void Update(const unsigned long &CurrentTime) noexcept
    {
        const auto blinkIntervalProgress = CurrentTime % BLINK_INTERVAL;
        const auto currentIndex = blinkIntervalProgress / (BLINK_INTERVAL / 8);
        const auto ledState = 0x80 >> currentIndex & static_cast<uint8_t>(signalStyle_);
        digitalWrite(LED_PIN, ledState);
        lastTime_ = CurrentTime;
        LOG_LED(String("blinkStyle: ") + static_cast<int>(signalStyle_) + " :: currentIndex = blinkIntervalProgress / BLINK_INTERVAL :: " + currentIndex + " = " + blinkIntervalProgress + " / " + BLINK_INTERVAL + " - LED: " + ledState)
    }

    // Sets a status type to be displayed by the led
    void ShowStatus(Signal SignalStyle)
    {
        signalStyle_ = SignalStyle;
        LOG_LED(String("ShowStatus: ") + static_cast<int>(signalStyle_))
    }

private:
    Signal signalStyle_;
    unsigned long lastTime_;
};

#endif
