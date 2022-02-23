#ifndef __RTC_HPP
#define __RTC_HPP

#include <DS3231.h>

#include "settings.hpp"

// Combine the mussy DS3231 library code into somewhot working with all needed implementations
class RTC : public DS3231, public RTClib
{
  public:
    RTC() : ds3231_{new DS3231()} { Wire.begin(); }

    ~RTC() { delete ds3231_; }

    // Returns the weekdays with the corresponding day bit set:
    // MTWTFSS == 0 000 0000
    // 0000 0001 == sunday
    // 0100 0000 == monday
    byte DayOfWeek() const noexcept
    {
        // Get the day of the week, starting on sunday
        // 1 = sunday, 7 = saturday
        return static_cast<uint8_t>(1) << (ds3231_->getDoW() - 1);
    }

  private:
    DS3231* ds3231_;
};

class Clock final
{
  public:
    // State the timer can suggest to the machine
    enum class State
    {
        Off,  // Timer says machine should be off
        On    // Timer wants the machine to be on
    };

    Clock(uint8_t NumberOfAvailableTimers = 1)
        : rtc_{new RTC()},
          turnOffAfterDuration_{0},
          turnOnAt_{0},
          turnOffAt_{0},
          days_{0},
          hasNewState_{false},
          state_{State::Off}
    {
    }

    ~Clock() { delete rtc_; };

    void Update() noexcept
    {
        const auto oldState = state_;
        const auto dateTime = rtc_->now();
        const auto unixTime = dateTime.unixtime();

        uint8_t weekday = rtc_->DayOfWeek();

        LOG_CLOCK(String("Clock update: Current: ") + dateTime.hour() + ":" + dateTime.minute() +
                  " timer1On: " + GetHours(turnOnAt_) + ":" + GetMinutes(turnOnAt_) +
                  " -- unixtime >= duration: " + unixTime + ">=" + turnOffAfterDuration_)

        // First the on stages, then the off stages to overwrite on stage.
        // Assumption:
        // One can only turn the machine off after it was turned on.
        // It is not possible to turn a machine off and on later in the same day.
        if (state_ != State::On && (weekday & days_) && dateTime.hour() >= GetHours(turnOnAt_) &&
            dateTime.minute() >= GetMinutes(turnOnAt_))
        {
            LOG_CLOCK(String("Turn on at: ") + dateTime.hour() + ":" + dateTime.minute() +
                      " >= " + GetHours(turnOnAt_) + ":" + GetMinutes(turnOnAt_))
            state_ = State::On;
            hasNewState_ = true;
        }

        if (turnOffAfterDuration_ != 0 && unixTime >= turnOffAfterDuration_)
        {
            LOG_CLOCK(String("Turn off after duration: ") + turnOffAfterDuration_ + " >= " + unixTime)
            turnOffAfterDuration_ = 0;
            state_ = State::Off;
            hasNewState_ = true;
        }
        // If the machine should only turn on, turnOffAt_ is 0.
        if (turnOffAt_ != 0 && state_ != State::Off && (weekday & days_) && dateTime.hour() >= GetHours(turnOffAt_) &&
            dateTime.minute() >= GetMinutes(turnOffAt_))
        {
            // Check that off timer comes after the on timer, else ignore it
            if (turnOffAt_ > turnOnAt_)
            {
                LOG_CLOCK(String("Turn off at: ") + dateTime.hour() + ":" + dateTime.minute() +
                          " >= " + GetHours(turnOffAt_) + ":" + GetMinutes(turnOffAt_))
                state_ = State::Off;
                hasNewState_ = true;
            }
            else
            {
                LOG_CLOCK("Turn off time lies before turn on time and is therefore ignored")
            }
        }

        // Check if a possibly triggered timer event is new
        // hasNewState_ = oldState != state_;
    }

    void SetDays(const uint8_t Days)
    {
        LOG_CLOCK(String("Clock got new timer days: ") + Days)
        days_ = Days;
    }

    // Duration in seconds to turn the machine off.
    // Input changed to minutes in vbm.cpp; durationtimer:2 --> 2 * 60 --> Duration = 120 s)
    void TurnOffIn(unsigned long int Duration) noexcept
    {
        LOG_CLOCK(String("Clock got new off duration time: ") + Duration + " s")
        turnOffAfterDuration_ = rtc_->now().unixtime() + Duration;
    }

    // Time to turn the machine on as minutes from midnight
    void TurnOnAt(unsigned long int MinutesFromMidnight) noexcept
    {
        // trunkate input to at least midnight
        constexpr const auto midnight = 24 * 60;  // TODO: move to cpp static member
        if (MinutesFromMidnight > midnight)
        {
            LOG_CLOCK("Invalid turn on input; truncate to midnight")
            MinutesFromMidnight = midnight;
        }
        LOG_CLOCK(String("Clock got new turn on time: ") + MinutesFromMidnight)
        turnOnAt_ = MinutesFromMidnight;
    }

    // Time to turn the machine off as minutes from midnight
    void TurnOffAt(unsigned long int MinutesFromMidnight) noexcept
    {
        // trunkate input to at least midnight
        constexpr const auto midnight = 24 * 60;  // TODO: move to cpp static member
        if (MinutesFromMidnight > midnight)
        {
            LOG_CLOCK("Invalid turn off input; truncate to midnight")
            MinutesFromMidnight = midnight;
        }
        LOG_CLOCK(String("Clock got new turn off time: ") + MinutesFromMidnight)
        turnOffAt_ = MinutesFromMidnight;
    }

    // Get the current timer state
    State State() const noexcept { return state_; }

    // Gets whether the clock timer state has changed
    bool HasNewState() noexcept
    {
        if (hasNewState_)
        {
            LOG_CLOCK(String("Clock has a new state: ") + static_cast<int>(state_))
        }
        return hasNewState_;
    }

  private:
    inline unsigned long int GetHours(unsigned long int MinutesFromMidnight)
    {
        return (MinutesFromMidnight - MinutesFromMidnight % 60) / 60;
    }

    inline unsigned long int GetMinutes(unsigned long int MinutesFromMidnight) { return MinutesFromMidnight % 60; }

    RTC* rtc_;
    enum State state_;

    bool hasNewState_;

    unsigned long int turnOffAfterDuration_;
    uint8_t days_;
    unsigned long int turnOnAt_;
    unsigned long int turnOffAt_;
};

#endif
