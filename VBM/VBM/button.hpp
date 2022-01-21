#ifndef __BUTTON_HPP
#define __BUTTON_HPP

#include "settings.hpp"

// A software debounced button that registers click and long clicks.
class Button
{
public:
    enum class Command
    {
        Error = -1, // something unexpected happened
        Nothing,    // nothing was pressed or released
        Click,      // pressed once, short
        ShortPress, // pressed and held for BUTTON_PRESS_SHORT seconds
        LongPress   // pressed and held for BUTTON_PRESS_LONG seconds
    };

    // Do not move constructor to cpp!
    Button()
        : registeredButtonCommand_{Command::Nothing}, alreadyTriggered_{false}, timeButtonPressed_{0}, timeButtonReleased_{0}, buttonCommandWasProcessed_{true}, startupDelay_{1000}

    {
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        digitalRead(BUTTON_PIN);
        attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Button::InterruptButton, CHANGE);
    }

    void Update() noexcept;

    Command RegisteredButtonPress() noexcept;

private:
    static void InterruptButton()
    {
        const auto fallingIfLow = digitalRead(BUTTON_PIN);
        if (fallingIfLow == LOW || !interruptButtonPressed_)
        {
            interruptButtonPressed_ = true;
            interruptButtonReleased_ = false;
            // LOG_BUTTON(String("XXXXXXXXXXXXXXXXXXXXXXXXXX == ") + fallingIfLow)
        }
        else
        {
            // interruptButtonPressed_ = false;
            interruptButtonReleased_ = true;
            // LOG_BUTTON(String("YYYYYYYYYYYYYYYYYYYYYYYYYY == ") + fallingIfLow)
        }
    }

    Command registeredButtonCommand_;
    static volatile bool interruptButtonPressed_;
    static volatile bool interruptButtonReleased_;

    unsigned long timeButtonPressed_;
    unsigned long timeButtonReleased_;
    const unsigned long startupDelay_;

    bool buttonCommandWasProcessed_;

    bool alreadyTriggered_;
};

#endif
