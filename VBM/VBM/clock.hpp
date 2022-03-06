#ifndef __RTC_HPP
#define __RTC_HPP

#include <time.h>

#include <DS3231.h>

#include "settings.hpp"

// Combine the mussy DS3231 library code into somewhot working with all needed implementations
class RTC : public DS3231, public RTClib
{
  public:
    RTC() : ds3231_{new DS3231()}
    {
        Wire.begin();
        // Always set to 24 hour format
        ds3231_->setClockMode(false);
    }

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

    Clock(uint8_t NumberOfAvailableTimers = 1);

    ~Clock();

    void Update() noexcept;

    // Get the weekdays the timer is active, represented as byte with sunday as lsb
    uint8_t Days() const noexcept { return days_; }

    // Sets the weekdays the timer is active, represented as byte with sunday as lsb
    void SetDays(const uint8_t Days)
    {
        LOG_CLOCK(String("Clock got new timer days: ") + Days)
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
    unsigned long int UnixTime() const noexcept
    {
        // return rtc_->now().unixtime();

        struct tm time;
        time.tm_year = rtc_->now().year() - 1900;  // Year - 1900
        time.tm_mon = rtc_->now().month() - 1;     // Month, where 0 = jan
        time.tm_mday = rtc_->now().day();          // Day of the month
        time.tm_hour = rtc_->now().hour();
        time.tm_min = rtc_->now().minute();
        time.tm_sec = rtc_->now().second();
        time.tm_isdst = -1;  // Is DST on? 1 = yes, 0 = no, -1 = unknown
        const time_t unixTime = mktime(&time) + UNIX_OFFSET;
        LOG_CLOCK(String("Getting unix time from DS3231; current time: ") + (rtc_->now().year()) + "/" +
                  (rtc_->now().month()) + "/" + rtc_->now().day() + " -- " + rtc_->now().hour() + ":" +
                  rtc_->now().minute() + ":" + rtc_->now().second() + "; == " + unixTime + "s")
        return unixTime;
    }

    void SetTimeFromUnixTime(unsigned long int CurrentUnixTime)
    {
        struct tm epoch;
        time_t timeFrom2k = CurrentUnixTime - UNIX_OFFSET;
        memcpy(&epoch, gmtime(&timeFrom2k), sizeof(struct tm));
        LOG_CLOCK(String("Set App time from ") + CurrentUnixTime + " over 30a unix offset: " + timeFrom2k +
                  " to DS3231: " + (epoch.tm_year + 1900) + '/' + (epoch.tm_mon + 1) + '/' + epoch.tm_mday + " - " +
                  epoch.tm_hour + ':' + epoch.tm_min + ':' + epoch.tm_sec + " weekday: " + (epoch.tm_wday + 1))
        rtc_->setYear(epoch.tm_year + 1900 - 48);  // time from 1900 // TODO: Write own DS3231 lib as this one seems really messed up
        rtc_->setMonth(epoch.tm_mon + 1);     // 0 indexed
        rtc_->setDate(epoch.tm_mday);         // days start at one
        rtc_->setDoW(epoch.tm_wday + 1);      // we want the week to start at sunday represented by 1
        rtc_->setHour(epoch.tm_hour);
        rtc_->setMinute(epoch.tm_min);
        rtc_->setSecond(epoch.tm_sec);
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
