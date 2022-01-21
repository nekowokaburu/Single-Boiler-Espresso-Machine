#ifndef __COMMUNICATOR_HPP
#define __COMMUNICATOR_HPP

#include "settings.hpp"

#include <EEPROM.h>

// **************************************************************************************************
// **************************************************************************************************
// Unfinished, this could also collect all data and map them to processing, after a chunk was used,
// remove the part from the string, keep fixed size sting for memory reasons.
// **************************************************************************************************
// **************************************************************************************************

// Opens a serial connection and sends/retrieves data
class Communicator
{
public:
    Communicator()
    {
        Serial.begin(115200);
        while (!Serial)
            ;
    }

    void Update() noexcept
    {
        // Create an empty string to store messages from Android. Do this in loop so nothing happens until new message received because it is reset here!
        String received_message;

        while (Serial.available())
        {                                            // keep reading bytes while they are still more in the buffer
            received_message += (char)Serial.read(); // read byte, convert to char, and append it to string
        }
        if (received_message.length())
        { // if string is not empty do the following
            LOG_COMM(String("Received Message: ") + received_message);
            /// Save values to eeprom
            if (received_message.startsWith("m"))
            {
                EEPROM.write(0, static_cast<uint8_t>(KP));
                EEPROM.write(1, static_cast<uint8_t>(KI));
                EEPROM.write(2, static_cast<uint8_t>(KD));
                // EEPROM.write(3, static_cast<uint8_t>(SETPOINT)); // This is a member now! Fix this!
                UpdateApp();
            }

            if (received_message.startsWith("p"))
            {
                KP = received_message.substring(1).toDouble();
                Serial.println(String("p") + KP);
            }

            if (received_message.startsWith("i"))
            {
                KI = received_message.substring(1).toDouble();
                Serial.println(String("i") + KI);
            }

            if (received_message.startsWith("d"))
            {
                KD = received_message.substring(1).toDouble();
                Serial.println(String("d") + KD);
            }

            if (received_message.startsWith("s"))
            {
                // SETPOINT = received_message.substring(1).toDouble(); // Member now!!! Fix this!
                // Serial.println(String("s") + SETPOINT);
            }

            /// On connection, update App with last used pressure values from eeprom
            if (received_message.startsWith("u"))
            {
                UpdateApp();
            }
        }
    }

private:
    // Update App parameteres on startup or after parameter change
    void UpdateApp(void)
    {
        // Serial.println(String("p") + KP + "i" + KI + "d" + KD + "s" + SETPOINT);
        Serial.println(String("p") + KP);
        Serial.println(String("i") + KI);
        Serial.println(String("d") + KD);
        // Serial.println(String("s") + SETPOINT); // Member now! Fix this!!!
    }
};

#endif
