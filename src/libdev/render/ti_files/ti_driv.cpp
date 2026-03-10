#include "ctl/CountedPtr.hpp"
#include "ctl/CountedPtr.ctp"

#include "ctl/List.hpp"
#include "ctl/List.ctp"

#include "render/Driver.hpp"

// class RenDriver;

void W4dDummyFunctionDriv()
{
    static CtlCountedPtr<RenDriver> dummyDriverPtr;

    static ctl_list<CtlCountedPtr<RenDriver>> dummyDrivers;
}
