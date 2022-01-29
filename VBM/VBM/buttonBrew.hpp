#ifndef __BUTTON_BREW_HPP
#define __BUTTON_BREW_HPP

#include "settings.hpp"

// A software debounced button that registers click and long clicks.
class ButtonBrew final
{
public:
    ButtonBrew()
    {
        pinMode(BUTTON_PIN_BREW, INPUT_PULLUP);
    }

    bool IsPressed() const noexcept
    {
        const bool state = !digitalRead(BUTTON_PIN_BREW);
        LOG_BUTTON_BREW(String("Brew button state: ") + state)
        return state;
    }
};

#endif
