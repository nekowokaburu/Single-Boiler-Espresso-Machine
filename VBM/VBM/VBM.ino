#include "vbm.hpp"

VBM* vbm = nullptr;

void setup()
{
    // Communicator will implement serial, but for now we skip this feature
    // Serial.begin(57600); // Baud rate of the Arduino Nano for BlueTooth serial upload with the old Atmel328p bootloader
    // while (!Serial)
    //     ;

    // // This helps to see if the device crashes or resets at some point.
    // Serial.println("Device started!");
    vbm = new VBM();
}

void loop()
{
    vbm->Update();
}
