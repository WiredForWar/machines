/*
 * P E R S I S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "world4d/Persistence.hpp"

// Note: unfortunately there are #include order dependencies due to forward
// declarations of templates.
#include "phys/Plans/RampAcceleration.hpp" // order dependencies

#include "world4d/Entity/Root.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Scene/Portal.hpp"
#include "world4d/Entity/GenericComposite.hpp"
#include "world4d/Entity/Generic.hpp"
#include "world4d/Entity/SpinSprite.hpp"
#include "world4d/Entity/Sprite3d.hpp"
#include "world4d/Scene/Camera.hpp"
#include "world4d/Entity/Custom.hpp"
#include "world4d/Scene/Shadow.hpp"
#include "world4d/Scene/Background.hpp"
#include "world4d/Plans/MeshPlan.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"
#include "world4d/Plans/ScalePlan.hpp"
#include "world4d/Plans/UVPlan.hpp"
#include "world4d/Plans/MaterialPlan.hpp"
#include "world4d/Scene/Light.hpp"
#include "world4d/Entity/Link.hpp"

#include "world4d/Materials/Internal/MultiTexture.hpp"
#include "world4d/Materials/Internal/UVData.hpp"
#include "world4d/Materials/Internal/MultiColour.hpp"
#include "world4d/Scene/Internal/LightData.hpp"
#include "world4d/Plans/Internal/PendingPlan.hpp"

#include "mathex/transf3d.hpp"

#include "phys/Plans/MotionPlan.hpp"

PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, PhysMotionPlanPtr);
PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, W4dMeshPlanPtr);
PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, W4dVisibilityPlanPtr);
PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, W4dScalePlanPtr);
PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, W4dUVPlanPtr);
PER_DEFINE_PERSISTENT_T1(W4dPendingPlan, W4dMaterialPlanPtr);

// static
W4dPersistence& W4dPersistence::instance()
{
    static W4dPersistence instance_;

    return instance_;
}

W4dPersistence::W4dPersistence()
{
    PER_REGISTER_DERIVED_CLASS(W4dRoot);
    PER_REGISTER_DERIVED_CLASS(W4dLink);
    PER_REGISTER_DERIVED_CLASS(W4dDomain);
    PER_REGISTER_DERIVED_CLASS(W4dGeneric);
    PER_REGISTER_DERIVED_CLASS(W4dGenericComposite);
    PER_REGISTER_DERIVED_CLASS(W4dSprite3d);
    PER_REGISTER_DERIVED_CLASS(W4dSpinSprite);
    PER_REGISTER_DERIVED_CLASS(W4dPortal);
    PER_REGISTER_DERIVED_CLASS(W4dCamera);
    PER_REGISTER_DERIVED_CLASS(W4dCustom);
    // PER_REGISTER_DERIVED_CLASS( W4dLight );
    PER_REGISTER_DERIVED_CLASS(W4dUniformLight);
    PER_REGISTER_DERIVED_CLASS(W4dDirectionalLight);
    PER_REGISTER_DERIVED_CLASS(W4dPointLight);
    PER_REGISTER_DERIVED_CLASS(W4dShadowProjected2d);
    PER_REGISTER_DERIVED_CLASS(W4dShadowFixed);
    PER_REGISTER_DERIVED_CLASS(W4dCycleMultiTextureData);
    PER_REGISTER_DERIVED_CLASS(W4dUVTranslateData);
    PER_REGISTER_DERIVED_CLASS(W4dMultiColourPulseData);
    PER_REGISTER_DERIVED_CLASS(W4dLightData);
}

W4dPersistence::~W4dPersistence()
{
}

void W4dPersistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const W4dPersistence& t)
{

    o << "W4dPersistence " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "W4dPersistence " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End PERSIST.CPP **************************************************/
