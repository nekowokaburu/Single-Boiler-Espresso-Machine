#ifndef __HEATER_HPP
#define __HEATER_HPP

#include <Adafruit_MAX31865.h>
// Get this library here: https://github.com/nekowokaburu/StuPID
// and copy it to your library directory. Maybe you'll need to update the
// library manager in order to find it.
#include <StuPID.hpp>

#include "settings.hpp"

class Heater final
{
  public:
    enum class State
    {
        Off,
        BrewTemp,
        SteamTemp
    };

    Heater();

    ~Heater();

    double CurrentTemperature() const noexcept
    {
      return currentTemperature_;
    }

    // Switch heater state and temperature regulation based on predefined values
    void SetHeaterTo(State HeaterState) noexcept;

    // Gets whether the heater reached the heating range once
    bool IsReady() noexcept;

    // Update heater management, must be called in a loop
    void Update() noexcept
    {
        UpdateTemperature();
        Boiler();
    }

  private:
    // Update the thermocouple readout
    void UpdateTemperature();

    // Computes if the heater should be on or off for this loop cycle
    void Boiler(void);

    State heaterState_;
    double currentTemperature_;
    double setpoint_;
    Adafruit_MAX31865* thermocouple_;
    StuPIDRelay* pid_;
    bool relayState_;
    unsigned long windowStartTime_;
    bool isReady_;
};

#endif
