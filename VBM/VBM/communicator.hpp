#ifndef __COMMUNICATOR_HPP
#define __COMMUNICATOR_HPP

#include "eepromMemory.hpp"
#include "settings.hpp"

// Opens a serial connection and sends/retrieves data
class Communicator
{
  public:
    // Communication commands
    enum class Command
    {
        None = 0,
        TurnOff,
        TurnOn,
        UpdateSetpointBrew,
        UpdateSetpointSteam,
        DurationTimer,  // duration in seconds when to turn the machine off starting now
        DaysTimer1,     // weekdays timer 1 should be active
        Timer1On,       // time when to turn the machine on in minutes from midnight
        Timer1Off       // time when to turn the machine off in minutes from midnight
    };

    Communicator();

    // Gets the last received command
    Command Command() noexcept;

    // Gets the last received value as the AVRs biggest supported type
    double Value() noexcept;

    // Update loop checks for a new message and retrieves it completely in one go.
    void Update() noexcept;

    // Send a message
    void SendMessage(String Message) const noexcept;

  private:
    // TODO: try catch this for any errors and return success for setting command or not
    // Gets the value from a message after the ':' and casts to double
    void SetValueFromMessage(const String& Message);

    String receivedMessage_;
    enum Command receivedCommand_;
    double receivedValue_;
};

#endif
