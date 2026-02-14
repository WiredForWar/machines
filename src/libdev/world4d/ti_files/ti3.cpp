#define _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/Map.hpp"
#include "String.hpp";

W4dDummyFunction3()
{
    ctl_map<string, uint, less<string>> dummySoundIDMap;
}

// #include "ctl/Vector.hpp"
// #include "ctl/Vector.ctp"
// #include "ctl/CountedPtr.hpp"
// #include "ctl/CountedPtr.ctp"
//
// #include "phys/Plans/MotionPlan.hpp"
// #include "world4d/Plans/MaterialPlan.hpp"
// #include "world4d/Plans/VisibilityPlan.hpp"
// #include "world4d/Tracking/ObjectTracker.hpp"
//
//
// W4dDummyFunction3()
//{
//     static CtlCountedPtr<W4dMaterialPlan> dummy;
//     static CtlCountedPtr<W4dVisibilityPlan> dummy1;
//     static CtlCountedPtr<W4dObjectTracker> dummy2;
//     static ctl_vector<size_t> dummySize_t;
// }