#include "vbm.hpp"

VBM* vbm = nullptr;

void setup()
{
    vbm = new VBM();
}

void loop()
{
    vbm->Update();
}
