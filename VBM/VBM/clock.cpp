#include "clock.hpp"

// Time from 00:00 to 24:00 which is our max timer value in minutes for turn on and off
static constexpr const auto MIDNIGHT = 24 * 60 - 1;

Clock::Clock(uint8_t NumberOfAvailableTimers)
    : rtc_{new RTC()},
      turnOffAfterDuration_{0},
      turnOnAt_{0},
      turnOffAt_{0},
      timerFiredOnceForTheDay_{0},
      days_{0},
      hasNewState_{false},
      state_{State::Off}
{
}

Clock::~Clock()
{
    delete rtc_;
}

void Clock::Update() noexcept
{
    const auto oldState = state_;
    const auto dateTime = rtc_->now();
    const auto unixTime = dateTime.unixtime();

    uint8_t weekday = rtc_->DayOfWeek();

    // Reset timer if day changes
    if (timerFiredOnceForTheDay_ != 0 && timerFiredOnceForTheDay_ != weekday)
        timerFiredOnceForTheDay_ = 0;

    // LOG_CLOCK(String("Clock update: Current: ") + dateTime.hour() + ":" + dateTime.minute() + " timer1On: " +
    //           GetHours(turnOnAt_) + ":" + GetMinutes(turnOnAt_) + " -- unixtime >= duration: " + unixTime +
    //           ">=" + turnOffAfterDuration_ + " -- secondsUNIX: " + (dateTime.unixtime() - UNIX_OFFSET))

    bool wouldTurnOff = false;
    // If the machine should only turn on, turnOffAt_ is 0.
    if (timerFiredOnceForTheDay_ && turnOffAt_ != 0 && (weekday & days_) && dateTime.hour() >= GetHours(turnOffAt_) &&
        dateTime.minute() >= GetMinutes(turnOffAt_))
        wouldTurnOff = true;

    // First the on stages, then the off stages to overwrite on stage.
    // Assumption:
    // One can only turn the machine off after it was turned on.
    // It is not possible to turn a machine off and on later in the same day.
    if (!timerFiredOnceForTheDay_ && state_ != State::On && turnOnAt_ != 0 && (weekday & days_) &&
        dateTime.hour() >= GetHours(turnOnAt_) && dateTime.minute() >= GetMinutes(turnOnAt_))
    {

        // Machine would not turn off and
        // if the machine was manually turn off, it was before timer would turn it on, not after
        if (!wouldTurnOff)
        {
            LOG_CLOCK(String("Turn on at: ") + dateTime.hour() + ":" + dateTime.minute() +
                      " >= " + GetHours(turnOnAt_) + ":" + GetMinutes(turnOnAt_))
            state_ = State::On;
            hasNewState_ = true;
            timerFiredOnceForTheDay_ = weekday;
        }
    }

    if (turnOffAfterDuration_ != 0 && unixTime >= turnOffAfterDuration_)
    {
        LOG_CLOCK(String("Turn off after duration: ") + turnOffAfterDuration_ + " >= " + unixTime)
        turnOffAfterDuration_ = 0;
        state_ = State::Off;
        hasNewState_ = true;
    }
    if (wouldTurnOff && state_ != State::Off)
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
}

void Clock::SetTurnOffIn(unsigned long int Duration) noexcept
{
    LOG_CLOCK(String("Clock got new off duration time: ") + Duration + " s")
    turnOffAfterDuration_ = rtc_->now().unixtime() + Duration;
}

void Clock::SetTurnOnAt(unsigned long int MinutesFromMidnight) noexcept
{
    // trunkate input to at least midnight
    if (MinutesFromMidnight > MIDNIGHT)
    {
        LOG_CLOCK("Invalid turn on input; truncate to midnight")
        MinutesFromMidnight = MinutesFromMidnight % MIDNIGHT;
    }
    LOG_CLOCK(String("Clock got new turn on time: ") + MinutesFromMidnight)
    turnOnAt_ = MinutesFromMidnight;
    timerFiredOnceForTheDay_ = 0;
    state_ = State::Off;
}

void Clock::SetTurnOffAt(unsigned long int MinutesFromMidnight) noexcept
{
    // trunkate input to at least midnight
    if (MinutesFromMidnight > MIDNIGHT)
    {
        LOG_CLOCK("Invalid turn off input; truncate to midnight")
        MinutesFromMidnight = MinutesFromMidnight % MIDNIGHT;
    }
    LOG_CLOCK(String("Clock got new turn off time: ") + MinutesFromMidnight)
    turnOffAt_ = MinutesFromMidnight;
}

bool Clock::HasNewState() noexcept
{
    if (hasNewState_)
    {
        LOG_CLOCK(String("Clock has a new state: ") + static_cast<int>(state_))
        hasNewState_ = false;
        return true;
    }
    return false;
}