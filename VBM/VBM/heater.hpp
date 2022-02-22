#ifndef __HEATER_HPP
#define __HEATER_HPP

#include <Adafruit_MAX31865.h>
#include <AutoPID.h>

#include "settings.hpp"

// Implementation of "AutoPIDRelay" which
// + Adds logging
// + Fixes bug where _lastPulseTime is uninitialized
// + Fixes bug for (currentTime - _lastPulseTime) > _windowSize where unsigned long diff can never be > _windowSize for
// larger _lastPulseTime than currentTime
// + Fixes bug where millis() updates faster than window increases (not sure if really possible though)
// This should be fixed with https://github.com/r-downing/AutoPID/pull/18
class VBMPID final : public AutoPID
{
  public:
    VBMPID(double* input, double* setpoint, bool* relayState, double windowSize, double Kp, double Ki, double Kd)
        : AutoPID(input, setpoint, &_output, 0, 1.0, Kp, Ki, Kd),
          _lastPulseTime{millis()},
          _input{input},
          _setpoint{setpoint}
    {
        _relayState = relayState;
        _windowSize = windowSize;
    };

    void run()
    {
        // update _output
        // LOG_HEATER("run auto pid")
        AutoPID::run();

        // set relay state
        // LOG_HEATER("update window time")
        const unsigned long currentTime = millis();

        while (currentTime > (_windowSize + _lastPulseTime))
        {
            _lastPulseTime += _windowSize;
            // LOG_HEATER(String(currentTime) + " > " + _lastPulseTime + " + " + _windowSize)
        }
        // Safety if output computes to high for input > setpoint.
        // TODO: This looks like a bug that should be fixed in PID probably.
        //  LOG_HEATER(String("update relay state: ") + millis() + " - " + _lastPulseTime + " < " + _output + " * " +
        //  _windowSize)
        LOG_HEATER(String("update relay state: ") + *_input + " >= " + *_setpoint + " -- output: " + _output)
        *_relayState = *_input >= *_setpoint ? false : ((millis() - _lastPulseTime) < (_output * _windowSize));
    }

  private:
    double* _input;
    double* _setpoint;
    bool* _relayState;
    unsigned long _windowSize, _lastPulseTime;
    double _output;
};

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
    VBMPID* pid_;
    bool relayState_;
    unsigned long windowStartTime_;
    bool isReady_;
};

#endif
