#include "button.hpp"

volatile bool Button::interruptButtonPressed_ = false;
volatile bool Button::interruptButtonReleased_ = false;

// Update version that only returns a state change after the button is released.
// This has a slightly slower response time and gives no feedback if the desired state is achieved.
//
// Button::Command Button::RegisteredButtonPress() noexcept
// {
//     buttonCommandWasProcessed_ = true;
//     LOG_BUTTON(String("Returning button state: ") + static_cast<int>(registeredButtonCommand_))
//     return registeredButtonCommand_;
// }
//
// void Button::Update() noexcept
// {
//     if (buttonCommandWasProcessed_)
//     {
//         registeredButtonCommand_ = Command::Nothing;
//         buttonCommandWasProcessed_ = false;
//     }

//     if (interruptButtonPressed_ && !alreadyTriggered_ && (millis() - timeButtonReleased_) > DEBOUNCE_DELAY)
//     {
//         alreadyTriggered_ = true;
//         timeButtonPressed_ = millis();
//         LOG_BUTTON("button pressed")
//     }

//     if ((millis() - timeButtonPressed_) > DEBOUNCE_DELAY && alreadyTriggered_ && interruptButtonReleased_)
//     {
//         if (interruptButtonReleased_ && alreadyTriggered_)
//         {
//             alreadyTriggered_ = false;
//             timeButtonReleased_ = millis();
//             LOG_BUTTON("button released")
//             interruptButtonPressed_ = false;
//             interruptButtonReleased_ = false;

//             const auto activationTime = (timeButtonReleased_ - timeButtonPressed_) / 1000;
//             LOG_BUTTON(String("Button press took ") + activationTime + " s");

//             if (activationTime < BUTTON_PRESS_SHORT)
//                 registeredButtonCommand_ = Command::Click;
//             else if (activationTime >= BUTTON_PRESS_SHORT && activationTime < BUTTON_PRESS_LONG)
//                 registeredButtonCommand_ = Command::ShortPress;
//             else if (activationTime >= BUTTON_PRESS_LONG)
//                 registeredButtonCommand_ = Command::LongPress;
//             else
//                 registeredButtonCommand_ = Command::Error;
//             LOG_BUTTON(String("Mapped button state to: ") + static_cast<int>(registeredButtonCommand_))
//         }
//     }
//     else if (interruptButtonPressed_ && interruptButtonReleased_)
//     {
//         interruptButtonPressed_ = false;
//         interruptButtonReleased_ = false;
//         registeredButtonCommand_ = Command::Click;
//         LOG_BUTTON(String("Mapped button state to: ") + static_cast<int>(registeredButtonCommand_))
//     }
// }

// -----------------------------------------------------------------------------------------------------------

// Update version that immediately reports on a state change before the button is release.
// This works for an espresso machine as the first state change will trigger heating up to steam or
// cooling down from steam to brew, the long press will turn the machine off. It makes no difference
// if in between the machine heats/cools for a few seconds.

Button::Command Button::RegisteredButtonPress() noexcept
{
    if (!buttonCommandWasProcessed_)
    {
        buttonCommandWasProcessed_ = true;
        LOG_BUTTON(String("Returning button state: ") + static_cast<int>(registeredButtonCommand_))
        return registeredButtonCommand_;
    }
    else
        return Command::Nothing;
}

void Button::Update() noexcept
{
    const unsigned long currentTime = millis();

    // Ignore button on startup to accompany for initial interrupt (errors)
    if (currentTime < startupDelay_)
        interruptButtonPressed_ = false;

    // Button is pressed
    if (interruptButtonPressed_ && !alreadyTriggered_ && (currentTime - timeButtonReleased_) > DEBOUNCE_DELAY)
    {
        alreadyTriggered_ = true;
        timeButtonPressed_ = currentTime;
        LOG_BUTTON("button pressed")
    }

    const auto timeSinceBtnPress = currentTime - timeButtonPressed_;

    if (timeSinceBtnPress > DEBOUNCE_DELAY && alreadyTriggered_ && interruptButtonReleased_)
    {
        LOG_BUTTON("button released")
        registeredButtonCommand_ = Command::Nothing;
        timeButtonReleased_ = currentTime;
        alreadyTriggered_ = false;
        interruptButtonPressed_ = false;
        interruptButtonReleased_ = false;

        const auto activationTime = (timeButtonReleased_ - timeButtonPressed_) / 1000;
        LOG_BUTTON(String("Button press took ") + activationTime + " s");

        if (activationTime < BUTTON_PRESS_SHORT)
        {
            registeredButtonCommand_ = Command::Click;
            buttonCommandWasProcessed_ = false;
        }
    }
    else if (interruptButtonPressed_ && interruptButtonReleased_)
    {
        interruptButtonPressed_ = false;
        interruptButtonReleased_ = false;
        buttonCommandWasProcessed_ = false;
        registeredButtonCommand_ = Command::Click;
    }
    else if (timeSinceBtnPress > DEBOUNCE_DELAY && alreadyTriggered_ && (timeSinceBtnPress / 1000) >= BUTTON_PRESS_LONG && static_cast<int>(registeredButtonCommand_) < static_cast<int>(Command::LongPress))
    {
        registeredButtonCommand_ = Command::LongPress;
        buttonCommandWasProcessed_ = false;
    }
    else if (timeSinceBtnPress > DEBOUNCE_DELAY && alreadyTriggered_ && (timeSinceBtnPress / 1000) >= BUTTON_PRESS_SHORT && static_cast<int>(registeredButtonCommand_) < static_cast<int>(Command::ShortPress))
    {
        registeredButtonCommand_ = Command::ShortPress;
        buttonCommandWasProcessed_ = false;
    }
}
