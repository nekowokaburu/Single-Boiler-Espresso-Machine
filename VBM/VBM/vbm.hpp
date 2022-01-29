#ifndef __VBM_HPP
#define __VBM_HPP

#include "button.hpp"
#include "buttonBrew.hpp"
#include "heater.hpp"
#include "led.hpp"

// TODO: This class could be created with the pins used
// Handles the machine states and the pump, combines heater, led and button
class VBM
{
  public:
    enum class State
    {
        Error = -1,
        Off = 0,
        Sleep,
        HeatingUpBrew,
        IdleBrew,
        HeatingUpSteam,
        IdleSteam,
        CoolingDown
    };

    VBM();

    ~VBM();

    // Updates the espressomachine's state and hanles the machine's functions
    void Update() noexcept;


  private:
    // Helper for debug state
    String StateToString() const noexcept;

    void HandleButton(Button::Command ButtonCommand) noexcept;
    // update the led status to be displayed based on the current machine state
    void HandleLED() noexcept;

    // Toggles the pump on/off
    void TogglePump() noexcept;

    // Turn pump off and set members correctly
    void TurnPumpOff() noexcept;

    Heater* heater_;
    LED* led_;
    Button* button_;
    ButtonBrew* buttonBrew_;

    State machineState_;
    unsigned long currentTime_;

    bool pumpOn_;
    bool wasBrewing_;
};

#endif
