#define _INSTANTIATE_TEMPLATE_CLASSES
#include "ctl/Vector.hpp"
#include <string.hpp>

void dummyTIObjectDataPop()
{
    static ctl_vector<string> dummyCtlVectorString;
}