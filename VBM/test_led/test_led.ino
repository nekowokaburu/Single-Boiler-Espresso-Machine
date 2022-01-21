constexpr const int LED_PIN = LED_BUILTIN;
const unsigned int BLINK_INTERVAL = 1000; // blink time in milliseconds, max 4 on/off in this time

#define LOG_LED(message) Serial.println(message);

#include "led.hpp"


LED led;

void setup()
{
    while (!Serial)
        ;
    Serial.begin(115200);

    led = LED();
}

void loop()
{
    led.ShowStatus(LED::Signal::Gallop),
    led.Update(millis());    
}
