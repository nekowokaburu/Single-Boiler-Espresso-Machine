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
        Timer1Off,      // time when to turn the machine off in minutes from midnight
        SetUnixTime,    // current time from App as unix time stamp
        UpdateApp,      // send all interesting parameters to the connected application
        Update          // general update to print some stuff
    };

    Communicator();

    // Gets the last received command
    Command Command() noexcept;

    // Gets the last received value
    template <class T>
    void Value(T& Value) const noexcept;

    // Update loop checks for a new message and retrieves it completely in one go.
    void Update() noexcept;

    // Send a message once, not in a loop
    void SendMessageOnce(String Message) const noexcept;

    // Sends a message once, formatted as command
    void SendMessageOnce(String Message, double Value) const noexcept;

    // TODO: Send message for use in loop, set how often this should update. Like update the temp for display
    // in the app, but update it once a second or mabye twice should be enough.

  private:
    String receivedMessage_;
    enum Command receivedCommand_;
};

template <class T>
void Communicator::Value(T& Value) const noexcept
{
    const auto numberStartsAfter = receivedMessage_.indexOf(String(":"));
    Value = static_cast<T>(strtoul(receivedMessage_.substring(numberStartsAfter + 1).c_str(), nullptr, 10));
    LOG_COMM(String("returning value from message: ") + receivedMessage_ + "; extracted value: " + Value)
}

#endif
