#include "communicator.hpp"

Communicator::Communicator() : receivedMessage_(""), receivedCommand_{Command::None}, receivedValue_{0}
{
    Serial.begin(57600);
    while (!Serial)
        ;

    // This helps to see if the device crashes or resets at some point.
    Serial.println("Communicator initialized!");
}

enum Communicator::Command Communicator::Command() noexcept
{
    LOG_COMM(String("returning received command: ") + static_cast<int>(receivedCommand_));
    auto const receivedCommandTmp = receivedCommand_;
    receivedCommand_ = Command::None;
    return receivedCommandTmp;
}

double Communicator::Value() noexcept
{
    LOG_COMM(String("returning received value: ") + receivedValue_)
    auto const receivedValueTmp = receivedValue_;
    receivedValue_ = 0;
    return receivedValueTmp;
}

void Communicator::Update() noexcept
{
    // Might be better to only do this once and rely on outer update loop?
    // Actually this should also only run if the Command and value were retrieved. Can be protected here or in VBM
    // class logic (currently the case).
    while (Serial.available())
        receivedMessage_ += (char)Serial.read();

    if (receivedMessage_.length())
    {  // if string is not empty do the following
        LOG_COMM(String("Received Message: ") + receivedMessage_)

        auto receivedMessageLower(receivedMessage_);
        receivedMessageLower.toLowerCase();
        receivedMessageLower.trim();

        if (receivedMessageLower == "turnon")
            receivedCommand_ = Command::TurnOn;
        else if (receivedMessageLower == "turnoff")
            receivedCommand_ = Command::TurnOff;
        else if (receivedMessageLower.startsWith(String("setpointbrew")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::UpdateSetpointBrew;
        }
        else if (receivedMessageLower.startsWith(String("setpointsteam")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::UpdateSetpointSteam;
        }
        else if (receivedMessageLower.startsWith(String("durationtimer")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::DurationTimer;
        }
        else if (receivedMessageLower.startsWith(String("daystimer1")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::DaysTimer1;
        }
        else if (receivedMessageLower.startsWith(String("timer1on")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::Timer1On;
        }
        else if (receivedMessageLower.startsWith(String("timer1off")))
        {
            SetValueFromMessage(receivedMessage_);
            receivedCommand_ = Command::Timer1Off;
        }

        receivedMessage_ = "";
    }
}

void Communicator::SendMessage(String Message) const noexcept
{
    LOG_COMM(String("Sending message: ") + Message)
    Serial.println(Message);
}

void Communicator::SetValueFromMessage(const String& Message)
{
    const auto numberStartsAfter = Message.indexOf(String(":"));
    receivedValue_ = Message.substring(numberStartsAfter + 1).toDouble();
    LOG_COMM(String("message: ") + Message + " extracted value: " + receivedValue_)
}
