#ifndef __EEPROMMEMORY_HPP
#define __EEPROMMEMORY_HPP

#include <EEPROM.h>

#include "settings.hpp"

class Eeprom
{
  public:
    // Add all available load/save types
    // Note: on AVR devices, double and float have the same precision (4 byte), same as unsigned long.
    // ATTENTION:
    // 1. Make sure to update the eepromIdx_ below accordingly!
    // 2. Make sure to update global variables in settings.hpp and settings.cpp
    // 3. Make sure to update VBM constructor with the initialization and the initial getter
    // 4. Probably want to set on communication the eeprom:
    //    a) update communicator for new parameters
    //    b) update VBM communication handler to also save the parameters to eeprom
    enum class Parameter
    {
        SetpointBrew = 0,  // double
        SetpointSteam,     // double
        Timer1Days,        // byte
        Timer1TurnOn,      // unsigned long int
        Timer1TurnOff,     // unsigned long int
    };

    // Saves a value from a known Parameter and manages sorting of size and position in EEPROM.
    // Returns true if all goes well, false otherwise.
    template <class T>
    bool Save(Parameter Parameter, T Value) noexcept;

    // Loads a value for an known parameter and manages sorting of size and position in EEPROM.
    // Returns true if all goes well, false otherwise. Value only changed on success.
    template <class T>
    bool Load(Parameter Parameter, T& Value) const noexcept;

  private:
    // Array to save the eeprom index and expected size of each savable/loadable value.
    // Increment the index by the sum of the previous sizes, set the size to the desired one for the new parameter.
    const uint8_t eepromIdx_[5][2] = {{0, 4},    // {{SetpointBrew, double},
                                      {4, 4},    //  {SetpointSteam, double},
                                      {8, 1},    //  {Timer1Days, byte}
                                      {9, 4},    //  {Timer1TurnOn, unsigned long int}
                                      {13, 4}};  //  {Timer1TurnOff, unsigned long int}}
};

template <class T>
bool Eeprom::Save(Parameter Parameter, T Value) noexcept
{
    const auto size = sizeof(T);
    LOG_EEPROM_MEMORY(String("Checking Save to eeprom; Idx: ") + eepromIdx_[static_cast<uint8_t>(Parameter)][0] +
                      ", expected size: " + eepromIdx_[static_cast<uint8_t>(Parameter)][1] + ", actual size: " + size)
    if (eepromIdx_[static_cast<uint8_t>(Parameter)][1] != size)
        return false;
    LOG_EEPROM_MEMORY("OK")

    byte* byteArray = reinterpret_cast<byte*>(&Value);
    const auto location = eepromIdx_[static_cast<uint8_t>(Parameter)][0];
    for (auto i = 0; i < size; i++)
    {
        EEPROM.write(location + i, byteArray[i]);
        LOG_EEPROM_MEMORY(String("Saving ") + byteArray[i] + " to eeprom location " + (location + i))
    }
    return true;
}

template <class T>
bool Eeprom::Load(Parameter Parameter, T& Value) const noexcept
{
    const auto size = sizeof(T);

    LOG_EEPROM_MEMORY(String("Checking Load from eeprom; Idx: ") + eepromIdx_[static_cast<uint8_t>(Parameter)][0] +
                      ", expected size: " + eepromIdx_[static_cast<uint8_t>(Parameter)][1] + ", actual size: " + size)
    if (eepromIdx_[static_cast<uint8_t>(Parameter)][1] != size)
        return false;
    LOG_EEPROM_MEMORY("OK; loaded")

    byte byteArray[size];
    const auto location = eepromIdx_[static_cast<uint8_t>(Parameter)][0];
    for (auto i = 0; i < size; i++)
    {
        byteArray[i] = EEPROM.read(location + i);
        LOG_EEPROM_MEMORY(String("Loading ") + byteArray[i] + " from eeprom location" + (location + i))
    }

    LOG_EEPROM_MEMORY_PRECISION(Value)
    Value = *reinterpret_cast<T*>(byteArray);
    return true;
}

#endif
