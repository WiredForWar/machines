#include "ctl/List.hpp"
#include "render/MaterialMap.hpp"

#include "ctl/List.ctp"
#include "ctl/set.ctp"
#include "ctl/Map.ctp"

class W4dD3DDriver;

W4dDummyFunction2()
{
    static ctl_list<const W4dD3DDriver*> driverList;
    static RenMaterialSet dummySet;
}
