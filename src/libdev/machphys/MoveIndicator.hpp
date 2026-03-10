#pragma once

#include "base/Persistence.hpp"

#include "world4d/Entity/Sprite3d.hpp"
#include "render/Material.hpp"
#include "render/render.hpp"
#include "world4d/Plans/VisibilityPlan.hpp"

// I'm attempting to adhere to the structure of other MachPhys*** classes to avoid unpleasant surprises.
class MachPhysMoveIndicator : public W4dSprite3d
{
public:
    MachPhysMoveIndicator(W4dEntity* pParent, const W4dTransform3d& localTransform, MATHEX_SCALAR size);
    ~MachPhysMoveIndicator() override;

    // NON-COPYABLE /////////////////////////////////////////////////////////////
    MachPhysMoveIndicator(const MachPhysMoveIndicator&) = delete;
    MachPhysMoveIndicator& operator=(const MachPhysMoveIndicator&) = delete;
    bool operator==(const MachPhysMoveIndicator&) = delete;
    /////////////////////////////////////////////////////////////////////////////

    static constexpr PhysRelativeTime DisplayTime { 1 };
    void startFadeOut(const PhysAbsoluteTime& startTime);

    /////////////////////////////////////////////////////////////////////////////
    friend std::ostream& operator<<(std::ostream& o, const MachPhysMoveIndicator& t);

    PER_MEMBER_PERSISTENT(MachPhysMoveIndicator);
    PER_FRIEND_READ_WRITE(MachPhysMoveIndicator);

private:
    static const W4dVisibilityPlanPtr& getFadeAwayPlan();
    static RenMaterial getMaterial();

    static RenTexture createTexture();
    static RenMaterial createMaterial();
};

PER_DECLARE_PERSISTENT(MachPhysMoveIndicator);
