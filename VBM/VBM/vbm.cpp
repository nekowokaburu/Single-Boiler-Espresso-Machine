#include "vbm.hpp"

VBM::VBM()
    : heater_(new Heater()),
      led_(new LED()),
      button_(new Button()),
      buttonBrew_(new ButtonBrew()),
      machineState_{State::Off},
      currentTime_{0},
      pumpOn_{false},
      wasBrewing_{false}
{
    pinMode(PUMP_SSR_PIN, OUTPUT);
    digitalWrite(PUMP_SSR_PIN, pumpOn_);

    if (DISABLE_HEATER)
        Serial.println(
            "WARNING: Heater disabled, code will run normally but machine will not heat up! Set DISABLE_HEATER to "
            "false for normal use!");
}

VBM::~VBM()
{
    delete heater_;
    delete led_;
    delete button_;
    delete buttonBrew_;
}

String VBM::StateToString() const noexcept
{
    switch (machineState_)
    {
        case State::Error:
            return "Error";
        case State::Off:
            return "Off";
        case State::Sleep:
            return "Sleep";
        case State::HeatingUpBrew:
            return "Heating up to brew temp";
        case State::IdleBrew:
            return "Idle brew";
        case State::HeatingUpSteam:
            return "Heating up to steam temp";
        case State::IdleSteam:
            return "Idle stream";
        case State::CoolingDown:
            return "Cooling down";

        default:
            return "Unknown state, please implement logging!";
            break;
    }
}

void VBM::Update() noexcept
{
    // call millis once for a repurposed current time
    currentTime_ = millis();

    // Brew lever state changed
    const bool isBrewing = buttonBrew_->IsPressed();
    if (machineState_ != State::Off && wasBrewing_ != isBrewing)
    {
        isBrewing ? digitalWrite(PUMP_SSR_PIN, HIGH) : digitalWrite(PUMP_SSR_PIN, LOW);
        pumpOn_ = isBrewing;
        wasBrewing_ = isBrewing;
    }

    // Update possible button input
    button_->Update();
    HandleButton(button_->RegisteredButtonPress());
    LOG_VBM(String("Got machine state from button: ") + static_cast<int>(machineState_) + " -- " + StateToString())

    // Do a calculation on the heater and bring the machine state in relation to the heater state
    heater_->Update();
    if (heater_->IsReady())
    {
        if (machineState_ == State::CoolingDown || machineState_ == State::HeatingUpBrew)
            machineState_ = State::IdleBrew;
        else if (machineState_ == State::HeatingUpSteam)
            machineState_ = State::IdleSteam;
        LOG_VBM(String("Heater updated machine state to: ") + static_cast<int>(machineState_))
    }

    // Update led state of the machine before led updates the "display"
    HandleLED();
    led_->Update(currentTime_);
}

void VBM::HandleButton(Button::Command ButtonCommand) noexcept
{
    switch (ButtonCommand)
    {
        case Button::Command::Nothing:
            // This does nothing for us
            break;
        case Button::Command::Click:
            if (machineState_ == State::Off || machineState_ == State::Sleep)
            {
                heater_->SetHeaterTo(Heater::State::BrewTemp);
                machineState_ = State::HeatingUpBrew;
            }
            else
                TogglePump();
            break;
        case Button::Command::ShortPress:
            if (machineState_ == State::Off)
            {
                heater_->SetHeaterTo(Heater::State::BrewTemp);
                machineState_ = State::HeatingUpBrew;
            }
            else if (machineState_ == State::HeatingUpSteam || machineState_ == State::IdleSteam)
            {
                heater_->SetHeaterTo(Heater::State::BrewTemp);
                machineState_ = heater_->IsReady() ? State::IdleBrew : State::CoolingDown;
            }
            else if (machineState_ == State::HeatingUpBrew || machineState_ == State::IdleBrew ||
                     machineState_ == State::CoolingDown)
            {
                heater_->SetHeaterTo(Heater::State::SteamTemp);
                machineState_ = heater_->IsReady() ? State::IdleSteam : State::HeatingUpSteam;
            }
            else
            {
                heater_->SetHeaterTo(Heater::State::Off);
                machineState_ = State::Error;
                LOG_VBM(String("Unkown machine state from button: ") + static_cast<int>(ButtonCommand) +
                        " with current state: " + static_cast<int>(machineState_))
            }
            break;
        case Button::Command::LongPress: {
            heater_->SetHeaterTo(Heater::State::Off);
            TurnPumpOff();
            machineState_ = State::Off;
        }
        break;

        case Button::Command::Error:
        default:
            heater_->SetHeaterTo(Heater::State::Off);
            machineState_ = State::Error;
            break;
    }
}

void VBM::HandleLED() noexcept
{
    switch (machineState_)
    {
        case State::Off:
            led_->ShowStatus(LED::Signal::Off);
            break;
        case State::Sleep:
            led_->ShowStatus(LED::Signal::Half);
        case State::HeatingUpBrew:
            led_->ShowStatus(LED::Signal::Quarter);
            break;
        case State::IdleBrew:
            led_->ShowStatus(LED::Signal::Whole);
            break;
        case State::HeatingUpSteam:
            led_->ShowStatus(LED::Signal::Sixteenth);
            break;
        case State::IdleSteam:
            led_->ShowStatus(LED::Signal::Eights);
            break;
        case State::CoolingDown:
            led_->ShowStatus(LED::Signal::Trab);
            break;

        default:
        case State::Error:
            led_->ShowStatus(LED::Signal::Gallop);
            break;
    }
}

void VBM::TogglePump() noexcept
{
    pumpOn_ = !pumpOn_;
    digitalWrite(PUMP_SSR_PIN, pumpOn_);
    LOG_VBM(String("Pump toggled: ") + pumpOn_)
}

void VBM::TurnPumpOff() noexcept
{
    pumpOn_ = false;
    digitalWrite(PUMP_SSR_PIN, pumpOn_);
    LOG_VBM("Pump turned off");
}
