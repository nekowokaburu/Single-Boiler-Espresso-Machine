# Single-Boiler-Espresso-Machine

Here you find a full implementation which runs a single boiler espresso machine such as a Vibiemme Domobar. You can run this code, if your machine has (or will have):
- Single brew/steam boiler
- Single button to operate the machine
- Single LED to show the current machine state
- If a lever switch starts the extraction, wire it directly to the pump, the pump can also be toggled by pushing the button once but this is rather to fill the boiler after steaming or run hot water through than to start the extraction, at least on a conventional machine
- I use a PT1000 thermocouple on the boiler with MAX31865

This also uses a PID library to get better temperature stability. All I/O can be set in [settings.hpp](VBM/VBM/settings.hpp). There you'll also find some logging macros for debugging purpose which can be turned on or off.

Future changes might include:
+ MPC on the heater
+ Bluetooth communication with a fitting Android App

### Machine usage
- Press once to turn the machine on if it is off or toggle the pump on/off if the machine is already on
- Press and hold for at least two seconds toggles brew and steam mode on the boiler
- Press and hold for at least five seconds turns the machine off

#### Machine states (see [VBM::State](VBM/VBM/vbm.hpp)):
-1 Machine has encountered an error and shuts the boiler down
0 Machine is turned off
1 Machine is in optional idle/sleep mode (like keeping a lower temp to preserve energy while enabling a faster heatup to ready)
2 Machine is heating up to brew temperature
3 Machine is att brew temperature
4 Machine is heating up to steam temperature
5 Machine is at steam temperature
6 Machine is cooling down

#### LED feedback
The led has different blink patterns (see [LED::Signal](VBM/VBM/led.hpp)). The most important are:
- LED Off --> The machine is off
- LED solid --> The machine is ready for brewing
- LED blinks very fast --> Machine ready to steam

All other blink states indicate heating to brew, heating to steam, cooling from steam to brew or error state, you will figure that out as you go.