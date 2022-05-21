#ifndef __VBM_HPP
#define __VBM_HPP

#include "button.hpp"
#include "buttonBrew.hpp"
#include "clock.hpp"
#include "communicator.hpp"
#include "eepromMemory.hpp"
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
    // Send all needed parameters for display purpose and time sync to the App host time.
    // Make sure this is called after possible eeprom load of the parameters.
    void UpdateApp() const noexcept
    {
        // Current machine state (on or off)
        Serial.println(String(">turnedon:") + static_cast<int>(machineState_ != State::Off));

        // Time from DS3231 as unix time
        const auto unixTime = clock_->UnixTime();
        Serial.println(String(">unixtime:") + unixTime);

        // Days of the week from all timers
        const auto timer1DoW = clock_->Days();
        Serial.println(String(">timer1dow:") + timer1DoW);

        // Timer values to turn the machine on
        const auto timer1On = clock_->TurnOnAt();
        Serial.println(String(">timer1on:") + timer1On);

        // Timer values to turn the machine off
        const auto timer1Off = clock_->TurnOffAt();
        Serial.println(String(">timer1off:") + timer1Off);

        Serial.println(String(">setpointbrew:") + SETPOINT_BREW_TEMP);
        Serial.println(String(">setpointsteam:") + SETPOINT_STEAM_TEMP);
    
        SendAdditionalParams();    
    }

    void SendAdditionalParams() const noexcept
    {
      const auto now = clock_->rtc_->now();
      Serial.println(String(">RTCUnix:") + now.unixtime());
      Serial.println(String(">weekday:") + now.dayOfTheWeek());
      Serial.println(String("clock:") + now.year() + "/" +  now.month() + "/" + now.day() + " - " + now.hour() + ":" + now.minute() + ":" + now.second());
      Serial.println(String(">temp:") + heater_->CurrentTemperature());
      // Serial.println(String(""));
    }

    // Helper for debug state
    String StateToString() const noexcept;

    // Manages button presses
    void HandleButton(Button::Command ButtonCommand) noexcept;

    // Manages the brew lever state change
    void HandleBrewLever(bool IsBrewing) noexcept;

    // update the led status to be displayed based on the current machine state
    void HandleLED() noexcept;

    // update and handle communication I/O
    void HandleCommunication(enum Communicator::Command Command) noexcept;

    // Toggles the pump on/off
    void TogglePump() noexcept;

    // Turn pump off and set members correctly
    void TurnPumpOff() noexcept;

    Heater* heater_;
    LED* led_;
    Button* button_;
    ButtonBrew* buttonBrew_;
    Communicator* communicator_;
    Eeprom* eeprom_;
    Clock* clock_;

    State machineState_;
    unsigned long currentTime_;

    bool pumpOn_;
    bool wasBrewing_;
};

#endif
