// These don't appear to be required?
// #include "ctl/Vector.hpp"
// #include "ctl/Vector.ctp"

#include "ctl/CountedPtr.hpp"
#include "ctl/CountedPtr.ctp"
#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

#include "world4d/Entity/CompositePlan.hpp"

W4dDummyFunction4()
{
    static CtlCountedPtr<W4dCompositePlan> dummyPtr;
    static vector<CtlCountedPtr<W4dCompositePlan>> dummy1;
}