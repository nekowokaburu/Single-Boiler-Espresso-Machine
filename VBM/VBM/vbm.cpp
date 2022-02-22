#include "vbm.hpp"

VBM::VBM()
    : heater_(new Heater()),
      led_(new LED()),
      button_(new Button()),
      buttonBrew_(new ButtonBrew()),
      communicator_(new Communicator()),
      machineState_{State::Off},
      eeprom_{new Eeprom()},
      clock_{new Clock()},
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

#ifdef INITIALIZE_EEPROM
    eeprom_->Save(Eeprom::Parameter::SetpointBrew, SETPOINT_BREW_TEMP);
    eeprom_->Save(Eeprom::Parameter::SetpointSteam, SETPOINT_STEAM_TEMP);
#endif

    // Load eeprom parameters with user set parameters as fallback if desired, for debugging sometimes not so smart
#if LOAD_INITIAL_PARAMETERS_FROM_EEPROM
    eeprom_->Load(Eeprom::Parameter::SetpointBrew, SETPOINT_BREW_TEMP);
    eeprom_->Load(Eeprom::Parameter::SetpointSteam, SETPOINT_STEAM_TEMP);
#endif
}

VBM::~VBM()
{
    delete heater_;
    delete led_;
    delete button_;
    delete buttonBrew_;
    delete communicator_;
    delete eeprom_;
    delete clock_;
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

    // Update communication input
    communicator_->Update();
    HandleCommunication(communicator_->Command());

    // Update timer dependent machine state
    clock_->Update();
    if (clock_->HasNewState())
    {
        if (clock_->State() == Clock::State::Off)
        {
            machineState_ = State::Off;
            LOG_VBM("VBM Timer turn machine off")
        }
        else if (clock_->State() == Clock::State::On)
        {
            LOG_VBM("VBM Timer turn machine on")
            if (machineState_ == State::Off || machineState_ == State::Sleep)
            {
                heater_->SetHeaterTo(Heater::State::BrewTemp);
                machineState_ = State::HeatingUpBrew;
            }
        }
    }

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
        case Button::Command::Click: {
            if (machineState_ == State::Off || machineState_ == State::Sleep)
            {
                heater_->SetHeaterTo(Heater::State::BrewTemp);
                machineState_ = State::HeatingUpBrew;
            }
            else
                TogglePump();
        }
        break;
        case Button::Command::ShortPress: {
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
                LOG_VBM(String("Unkown machine state from button: ") + static_cast<int>(ButtonCommand) +
                        " with current state: " + static_cast<int>(machineState_))
                heater_->SetHeaterTo(Heater::State::Off);
                machineState_ = State::Error;
            }
        }
        break;
        case Button::Command::LongPress: {
            heater_->SetHeaterTo(Heater::State::Off);
            TurnPumpOff();
            machineState_ = State::Off;
        }
        break;

        case Button::Command::Error:
        default: {
            heater_->SetHeaterTo(Heater::State::Off);
            machineState_ = State::Error;
        }
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

void VBM::HandleCommunication(enum Communicator::Command Command) noexcept
{
    switch (Command)
    {
        case Communicator::Command::TurnOn: {
            LOG_VBM(String("communication: TurnOn"))
            heater_->SetHeaterTo(Heater::State::BrewTemp);
            machineState_ = State::HeatingUpBrew;
        }
        break;
        case Communicator::Command::TurnOff: {
            LOG_VBM(String("communication: TurnOff"))
            heater_->SetHeaterTo(Heater::State::Off);
            machineState_ = State::Off;
        }
        break;
        case Communicator::Command::UpdateSetpointBrew: {
            const auto newSetpointBrew = communicator_->Value();
            LOG_VBM(String("communication: UpdateSetpointBrew:") + newSetpointBrew)
            SETPOINT_BREW_TEMP = newSetpointBrew;
            // Save the new setpoint to the eeprom, cast to be excactly clear what type we want!
            eeprom_->Save(Eeprom::Parameter::SetpointBrew, static_cast<float>(newSetpointBrew));
        }
        break;
        case Communicator::Command::UpdateSetpointSteam: {
            const auto newSetpointSteam = communicator_->Value();
            LOG_VBM(String("communication: UpdateSetpointSteam:") + newSetpointSteam)
            SETPOINT_STEAM_TEMP = newSetpointSteam;
            // Save the new setpoint to the eeprom, cast to be excactly clear what type we want!
            eeprom_->Save(Eeprom::Parameter::SetpointBrew, static_cast<float>(newSetpointSteam));
        }
        break;
        case Communicator::Command::DurationTimer: {
            const auto durationInMin = communicator_->Value();
            LOG_VBM(String("communication: Turn machine off in ") + durationInMin + " s")
            clock_->TurnOffIn(durationInMin * 60);
        }
        break;
        case Communicator::Command::DaysTimer1: {
            const uint8_t daysTimer1 = communicator_->Value();
            LOG_VBM(String("communication: Set days to ") + daysTimer1)
            clock_->SetDays(daysTimer1);
        }
        break;
        case Communicator::Command::Timer1On: {
            const auto timeFromMidnightInMinOn = communicator_->Value();
            LOG_VBM(String("communication: Turn machine on at ") +
                    static_cast<int>((timeFromMidnightInMinOn - static_cast<int>(timeFromMidnightInMinOn) % 60) / 60) +
                    ":" + static_cast<int>(timeFromMidnightInMinOn) % 60)
            clock_->TurnOnAt(timeFromMidnightInMinOn);
        }
        break;
        case Communicator::Command::Timer1Off: {
            const auto timeFromMidnightInMinOff = communicator_->Value();
            LOG_VBM(
                String("communication: Turn machine off at ") +
                static_cast<int>((timeFromMidnightInMinOff - static_cast<int>(timeFromMidnightInMinOff) % 60) / 60) +
                ":" + static_cast<int>(timeFromMidnightInMinOff) % 60)
            clock_->TurnOffAt(timeFromMidnightInMinOff);
        }
        break;
        default: {
            // Don't call the log heler as it would break the loop
            // Serial.println(String("HandleCommunication - not implemented command: ") + static_cast<int>(Command));
        }
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
