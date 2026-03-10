#define _INSTANTIATE_TEMPLATE_CLASSES

#include "base/base.hpp"

#include "pertest/master.hpp"

#include "pertest/tclass1.hpp"

// #include "ctl/CountedPtr.hpp"
// #include "ctl/CountedPtr.ctp"
// #include "ctl/Vector.hpp"
// #include "ctl/Vector.ctp"
// #include "ctl/MinMemoryVector.hpp"
// #include "ctl/MinMemoryVector.ctp"
// #include "ctl/FixedVector.hpp"
// #include "ctl/FixedVector.ctp"
// #include "ctl/List.hpp"
// #include "ctl/List.ctp"
// #include "pertest/class8.hpp"

void dummyFunction()
{
    //  static  CtlCountedPtr< int >            dummy1;
    //  static  CtlConstCountedPtr< int >       dummy1a;
    //  static  CtlCountedPtr< Class8 >         dummy2;
    //  static  CtlConstCountedPtr< Class8 >    dummy3;
    //     static  ctl_vector< Class8 >            dummy4;
    //     static  ctl_vector< int >               dummy5;
    //     static  ctl_fixed_vector< int >         dummy6;
    //     static  ctl_min_memory_vector< int >    dummy7;
    //     static  ctl_list< int >                 dummy8;

    static Master::TC1a dummy1;
}

PER_DEFINE_PERSISTENT_T2(TClass1, int, double);
