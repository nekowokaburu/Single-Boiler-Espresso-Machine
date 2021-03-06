#ifndef __RTC_HPP
#define __RTC_HPP

#include <time.h>

#include <RTClib.h>

#include "settings.hpp"

// Sun = days & 0000_0001
// Mon = days & 0000_0010
// Tue = days & 0000_0100
// Wed = days & 0000_1000
// Thu = days & 0001_0000
// Fri = days & 0010_0000
// Sat = days & 0100_0000
class Clock final
{
public:
    // State the timer can suggest to the machine
    enum class State
    {
        Off, // Timer says machine should be off
        On   // Timer wants the machine to be on
    };

    Clock(uint8_t NumberOfAvailableTimers = 1);

    ~Clock();

    void Update() noexcept;

    // Get the weekdays the timer is active, represented as byte with sunday as lsb
    uint8_t Days() const noexcept { return days_; }

    // Sets the weekdays the timer is active, represented as byte with sunday as lsb
    void SetDays(const uint8_t Days)
    {
        LOG_CLOCK(String("Clock got new timer days: ") + Days)
        if (Days & (1 << rtc_->now().dayOfTheWeek()))
        {
            timerFiredOnceForTheDay_ = 0;
            state_ = State::Off;
        }
        days_ = Days;
    }

    // Duration in seconds to turn the machine off.
    // Input changed to minutes in vbm.cpp; durationtimer:2 --> 2 * 60 --> Duration = 120 s)
    void SetTurnOffIn(unsigned long int Duration) noexcept;

    // Get the time to turn the machine on as minutes from midnight
    unsigned long int TurnOnAt() const noexcept { return turnOnAt_; }

    // Set the time to turn the machine on as minutes from midnight
    void SetTurnOnAt(unsigned long int MinutesFromMidnight) noexcept;

    // Get the time to turn the machine on as minutes from midnight
    unsigned long int TurnOffAt() const noexcept { return turnOffAt_; }

    // Time to turn the machine off as minutes from midnight
    void SetTurnOffAt(unsigned long int MinutesFromMidnight) noexcept;

    // Get the current timer state
    State State() const noexcept { return state_; }

    // Gets whether the clock timer state has changed
    bool HasNewState() noexcept;

    // Get the current unix time
    unsigned long int UnixTime() noexcept
    {
        return rtc_->now().unixtime();
    }

    void SetTimeFromUnixTime(unsigned long int CurrentUnixTime)
    {
        rtc_->adjust(DateTime(CurrentUnixTime));
    }

    RTC_DS3231 *rtc_;
private:
    inline unsigned long int GetHours(unsigned long int MinutesFromMidnight)
    {
        return (MinutesFromMidnight - GetMinutes(MinutesFromMidnight)) / 60;
    }

    inline unsigned long int GetMinutes(unsigned long int MinutesFromMidnight) { return MinutesFromMidnight % 60; }

    enum State state_;

    bool hasNewState_;

    unsigned long int turnOffAfterDuration_;
    uint8_t days_;
    unsigned long int turnOnAt_;
    unsigned long int turnOffAt_;
    uint8_t timerFiredOnceForTheDay_;
};

#endif
