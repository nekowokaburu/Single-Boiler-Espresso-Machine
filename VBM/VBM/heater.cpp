#include "heater.hpp"

Heater::Heater()
    : isReady_{false},
      heaterState_{State::Off},
      currentTemperature_{0},
      thermocouple_(new Adafruit_MAX31865(BOILER_TEMP_CS_PIN)),
      relayState_(false),
      windowStartTime_{millis()},
      setpoint_{0}
{
    pinMode(BOILER_SSR_PIN, OUTPUT);
    digitalWrite(BOILER_SSR_PIN, LOW);

    thermocouple_->begin(MAX31865_TYPE);

    pid_ = new VBMPID(&currentTemperature_, &setpoint_, &relayState_, WINDOW_SIZE, KP, KI, KD);
    // pid_->setBangBang(12);
    pid_->setTimeStep(WINDOW_SIZE);
}

Heater::~Heater()
{
    delete thermocouple_;
    delete pid_;
}

void Heater::SetHeaterTo(State HeaterState) noexcept
{
    // On state change, set to not ready
    if (heaterState_ != HeaterState)
        isReady_ = false;

    heaterState_ = HeaterState;
    switch (heaterState_)
    {
        case State::BrewTemp:
            setpoint_ = SETPOINT_BREW_TEMP;
            break;

        case State::SteamTemp:
            setpoint_ = SETPOINT_STEAM_TEMP;
            break;
        default:
        case State::Off:
            setpoint_ = 0;
            break;
    }
    LOG_HEATER(String("SetHeaterTo: ") + static_cast<int>(heaterState_) + " with setpoint: " + setpoint_)
}

bool Heater::IsReady() noexcept
{
    // Set isReady once for each stage
    if (!isReady_)
        isReady_ =
            ((setpoint_ - IS_READY_RANGE) < currentTemperature_) && ((setpoint_ + IS_READY_RANGE) > currentTemperature_)
                ? true
                : false;
    LOG_HEATER(String(">IsReady") + isReady_)
    return isReady_;
}

void Heater::UpdateTemperature()
{
    currentTemperature_ = thermocouple_->temperature(RNOMINAL, RREF);
    LOG_HEATER(String(">temperature:") + currentTemperature_)
    LOG_HEATER(String(">setpoint:") + setpoint_)
}

void Heater::Boiler(void)
{
    // For now I duplicated this code as the relayState_ will be switched from AutoPIDRelay
    if (heaterState_ == State::Off)
    {
        digitalWrite(BOILER_SSR_PIN, LOW);
        LOG_HEATER(String(">heater_ssr:0"));
    }
    else
    {
        pid_->run();
        if (!DISABLE_HEATER)
            digitalWrite(BOILER_SSR_PIN, relayState_);
        LOG_HEATER(String(">heater_ssr:") + relayState_);
    }
}
