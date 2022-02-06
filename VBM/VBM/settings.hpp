#ifndef __SETTINGS_HPP
#define __SETTINGS_HPP

#include <Arduino.h>

#define DISABLE_HEATER 0

#define DEBUG_COMM 0
#define DEBUG_BUTTON_SWITCH 0
#define DEBUG_BUTTON_BREW 0
#define DEBUG_HEATER 1
#define DEBUG_LED 0
#define DEBUG_VBM 0

#pragma region debug helper

#if DEBUG_COMM == 0
#define LOG_COMM(message)
#else
#define LOG_COMM(message) Serial.println(message);
#endif

#if DEBUG_BUTTON_SWITCH == 0
#define LOG_BUTTON_SWITCH(message)
#else
#define LOG_BUTTON_SWITCH(message) Serial.println(message);
#endif

#if DEBUG_BUTTON_BREW == 0
#define LOG_BUTTON_BREW(message)
#else
#define LOG_BUTTON_BREW(message) Serial.println(message);
#endif

#if DEBUG_HEATER == 0
#define LOG_HEATER(message)
#else
#define LOG_HEATER(message) Serial.println(message);
#endif

#if DEBUG_LED == 0
#define LOG_LED(message)
#else
#define LOG_LED(message) Serial.println(message);
#endif

#if DEBUG_VBM == 0
#define LOG_VBM(message)
#else
#define LOG_VBM(message) Serial.println(message);
#endif

#pragma endregion

#pragma region pins

// Use software SPI: CS, DI, DO, CLK
// Adafruit_MAX31865 thermocouple_ = Adafruit_MAX31865(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
constexpr const int BOILER_TEMP_CS_PIN = 10;  // CS/SS for hardware SPI
constexpr const int BOILER_SSR_PIN = 7;
constexpr const int PUMP_SSR_PIN = 9;
constexpr const int BUTTON_PIN_BREW =
    2;  // must be interrupt pin like 2 or 3 on Uno/Nano; connects to PIN and GND, for brew lever switch
constexpr const int BUTTON_PIN_SWITCH =
    3;  // must be interrupt pin like 2 or 3 on Uno/Nano; connects to PIN and GND, for manual switch
constexpr const int LED_PIN = 8;

#pragma endregion

#pragma region user variables

const constexpr double SETPOINT_BREW_TEMP = 100.0;
const constexpr double SETPOINT_STEAM_TEMP = 130.0;
const constexpr double MAX_TEMP = 135.0;  // max allowed temp on PID computation

// PID ******************************************************************************
// best precision: +-0.5 degree: 0.1, 0, 800, oscillates by +-0.5degree
const constexpr double KP = 0.05;  // 0.1; // 0.05 or so?
const constexpr double KI = 0;     // 0; // not even 0.01;, overshoots constantly rather 0.0003 or so
const constexpr double KD = 0.3;   // 2500; //90.53; // this was good with 0.53
constexpr const double WINDOW_SIZE = 50;

// Thermocouple *********************************************************************
#define MAX31865_TYPE MAX31865_2WIRE  // set to 3WIRE or 4WIRE as necessary
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 1000.0

#pragma endregion

#pragma region global program stuff

const uint8_t IS_READY_RANGE = 2;          // +/- this range signals heater ready state
const unsigned int BLINK_INTERVAL = 2500;  // blink time in milliseconds, max 4 on/off in this time

const uint8_t BUTTON_PRESS_SHORT = 2;  // time to register short button press in seconds
const uint8_t BUTTON_PRESS_LONG = 5;   // time to register long button press in seconds
const uint8_t DEBOUNCE_DELAY = 50;     // time to ignore button input in milliseconds

#pragma endregion

#endif
