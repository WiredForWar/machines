#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

#include "ctl/List.hpp"
#include "ctl/List.ctp"

#include "mathex/Transform3d.hpp"
#include "mathex/Point3d.hpp"

#include "world4d/Entity/Entity.hpp"

class W4dEntity;

W4dDummyFunction1()
{
    static ctl_vector<MexPoint3d> dummyPoints;
    ctl_vector<pair<W4dEntity*, MexTransform3d>> dummyStoredLinkPositions;
}
