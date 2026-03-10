#define _INSTANTIATE_TEMPLATE_CLASSES
#include "ctl/Map.hpp"
#include "mathex/Angle.hpp"

class EnvElevationColourTable;

void envDummyFunction3()
{
    ctl_map<MexDegrees, EnvElevationColourTable*, less<MexDegrees>> cluts1;
}
