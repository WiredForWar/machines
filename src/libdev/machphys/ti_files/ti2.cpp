/*
 * T I 9 . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

// #define _INSTANTIATE_TEMPLATE_CLASSES

#include "ctl/Vector.hpp"
#include "ctl/Vector.ctp"

#include "mathex/Transform3d.hpp"
#include "machphys/Effects/SmokePlume.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/FloatValuePlan.hpp"
#include "render/Texture.hpp"

class MachPhysEntrance;

void MachDummyFunction9()
{
    static MachPhysSmokePlume dummy2(NULL, MexTransform3d(), 0.0, 0.0, 0, 0.0, PUFF_1, 0.0);

    static ctl_vector<MachPhysEntrance*> dummy3;
    static ctl_vector<RenTexture> dummy4;
}

/* End TI9.CPP *****************************************************/
