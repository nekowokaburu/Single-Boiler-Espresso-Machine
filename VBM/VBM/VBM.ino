#include "vbm.hpp"
#include "button.hpp"

// #include "communicator.hpp"

VBM* vbm = nullptr;
// Communicator* btComm = nullptr;

void setup()
{
    // Communicator will implement serial, but for now we skip this feature
    // btComm = new Communicator();
    Serial.begin(115200);
    while (!Serial)
        ;

    vbm = new VBM();
}

void loop()
{
    // delay(100);
    vbm->Update();
}
