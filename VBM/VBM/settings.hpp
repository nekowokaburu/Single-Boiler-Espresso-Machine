#ifndef __SETTINGS_HPP
#define __SETTINGS_HPP

#include <Arduino.h>

// Global useful parameters to set for building and testing
#define DISABLE_HEATER 0  // default false; if true the heater will never actually turn on, all code paths run normally
#define DISABLE_PUMP 0    // default false; if true the pump will never actually turn on, all code paths run normally
// Load eeprom parameters as far as available with settings here as fallback
// ATTENTION: On a new board, this must at least be true ONCE to initialize the EEPROM to valid values!
#define LOAD_INITIAL_PARAMETERS_FROM_EEPROM 1  // Default true;

// Turn on/off debug information for each module
#define DEBUG_EEPROM_MEMORY 0
#define DEBUG_EEPROM_MEMORY_PRECISION 0
#define DEBUG_CLOCK 1
#define DEBUG_COMM 0
#define DEBUG_BUTTON_SWITCH 0
#define DEBUG_BUTTON_BREW 0
#define DEBUG_HEATER 0
#define DEBUG_LED 0
#define DEBUG_VBM 0

#pragma region debug helper definitions

// Always overwrite eeprom values on setting new parameters in this script
#if LOAD_INITIAL_PARAMETERS_FROM_EEPROM
#define INITIALIZE_EEPROM 0
#endif

#if DEBUG_EEPROM_MEMORY == 0
#define LOG_EEPROM_MEMORY(message)
#else
#define LOG_EEPROM_MEMORY(message) Serial.println(message);
#endif

#if DEBUG_EEPROM_MEMORY_PRECISION == 0
#define LOG_EEPROM_MEMORY_PRECISION(message)
#else
#define LOG_EEPROM_MEMORY_PRECISION(message) \
    Serial.println(message, 7);  // 6 to 7 digits total supported on AVR (float)
#endif

#if DEBUG_CLOCK == 0
#define LOG_CLOCK(message)
#else
#define LOG_CLOCK(message) Serial.println(message);
#endif

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

#pragma endregion debug helper definitions

#pragma region I / O pin settings

// Use software SPI: CS, DI, DO, CLK
// Adafruit_MAX31865 thermocouple_ = Adafruit_MAX31865(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
constexpr const int BOILER_TEMP_CS_PIN = A0;  // CS/SS for hardware SPI
constexpr const int BOILER_SSR_PIN = 3;
constexpr const int PUMP_SSR_PIN = 9;
constexpr const int BUTTON_PIN_BREW = 2;    // connects to PIN and GND, for brew lever switch
constexpr const int BUTTON_PIN_SWITCH = 7;  // connects to PIN and GND, for manual switch
constexpr const int LED_PIN = 8;

#pragma endregion I / O pin settings

#pragma region eeprom loadable / saveable user fallback variables
// Set fallback if it cannot be loaded from eeprom in settings.cpp
extern float SETPOINT_BREW_TEMP;
extern float SETPOINT_STEAM_TEMP;

#pragma endregion eeprom loadable / saveable user fallback variables

#pragma region user variables

const constexpr float MAX_TEMP = 135.0;  // max allowed temp on PID computation

// PID ******************************************************************************
// best precision: +-0.5 degree: 0.1, 0, 800, oscillates by +-0.5degree
const constexpr double KP = 0.08;
const constexpr double KI = 0.0001;
const constexpr double KD = 2.0;
constexpr const double WINDOW_SIZE = 3000.0;

// Thermocouple *********************************************************************
#define MAX31865_TYPE MAX31865_2WIRE  // set to 3WIRE or 4WIRE as necessary
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF 4300.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 1000.0

#pragma endregion user variables

#pragma region global program stuff

const uint8_t IS_READY_RANGE = 4;          // +/- this range signals heater ready state
const unsigned int BLINK_INTERVAL = 2500;  // blink time in milliseconds, max 4 on/off in this time

const uint8_t BUTTON_PRESS_SHORT = 2;  // time to register short button press in seconds
const uint8_t BUTTON_PRESS_LONG = 5;   // time to register long button press in seconds
const uint8_t DEBOUNCE_DELAY = 50;     // time to ignore button input in milliseconds

#pragma endregion global program stuff

#endif
