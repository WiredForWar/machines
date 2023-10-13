/*
 * S A T E L I T E . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "envirnmt/satelite.hpp"
#include "envirnmt/orbit.hpp"
#include "envirnmt/elevclut.hpp"
#include "envirnmt/elevmatp.hpp"
#include "world4d/generic.hpp"
#include "world4d/light.hpp"
#include "world4d/entyplan.hpp"
#include "world4d/manager.hpp"
#include "render/meshinst.hpp"
#include "render/mesh.hpp"
#include "mathex/point3d.hpp"
#include "system/pathname.hpp"

EnvSatellite::EnvSatellite(const string& n, EnvOrbit* orb)
    : matTable_(nullptr)
    , ambTable_(nullptr)
    , lightTable_(nullptr)
    , orbit_(orb)
    , name_(n)
    , light_(nullptr)
{
    PRE(orb);

    TEST_INVARIANT;
}

EnvSatellite::~EnvSatellite()
{
    TEST_INVARIANT;
    delete orbit_;
    delete lightTable_;
    delete ambTable_;
    delete matTable_;
}

void EnvSatellite::loadMesh(const SysPathName& envFile, EnvElevationColourTable* colours)
{
    // Use the satellite's name as the mesh name.
    W4dGeneric* entity = orbit_->movingEntity();
    entity->loadSingleMesh(envFile, name_);

    // If a colour table was given for the mesh, create an appropriate material plan.
    if (colours)
    {
        const W4dGeneric* constEntity = orbit_->movingEntity();
        matTable_ = colours;
        std::unique_ptr<RenMaterialVec> mats = constEntity->mesh().mesh()->materialVec();
        EnvElevationMaterialPlan* plan = new EnvElevationMaterialPlan(*(mats.get()), *this, *matTable_);
        entity->entityPlanForEdit().materialPlan(plan, W4dManager::instance().time());
    }
}

void EnvSatellite::setDirectionalLight(EnvElevationColourTable* clut)
{
    PRE(clut);
    lightTable_ = clut;

    // Create a light to represent the sun attached to whatever entity the
    // orbit is moving.
    light_ = new W4dDirectionalLight(orbit_->movingEntity(), MexVec3(1, 0, 0));
}

void EnvSatellite::setAmbientLight(EnvElevationColourTable* clut)
{
    ambTable_ = clut;
}

void EnvSatellite::update()
{
    if (light_ && lightTable_)
        light_->colour(lightTable_->colour(elevation()));
}

RenColour EnvSatellite::ambient() const
{
    if (ambTable_)
        return ambTable_->colour(elevation());
    else
        return RenColour::black();
}

EnvOrbit& EnvSatellite::orbit()
{
    TEST_INVARIANT;
    return *orbit_;
}

const EnvOrbit& EnvSatellite::orbit() const
{
    TEST_INVARIANT;
    return *orbit_;
}

MexRadians EnvSatellite::elevation() const
{
    TEST_INVARIANT;
    return orbit_->elevation();
}

W4dDirectionalLight* EnvSatellite::light()
{
    TEST_INVARIANT;
    return light_;
}

const W4dDirectionalLight* EnvSatellite::light() const
{
    TEST_INVARIANT;
    return light_;
}

void EnvSatellite::visible(bool setVisible)
{
    if (orbit_ and orbit_->movingEntity())
        orbit_->movingEntity()->visible(setVisible);
}

void EnvSatellite::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
    INVARIANT(orbit_ != nullptr);
    INVARIANT(implies(light_, lightTable_));
}

std::ostream& operator<<(std::ostream& o, const EnvSatellite& t)
{
    const MexTransform3d& xform = t.orbit_->movingEntity()->globalTransform();
    const MexPoint3d pos = xform.position();
    const MexDegrees el = t.elevation();

    const std::ios::fmtflags oldFlags = o.flags();
    const int oldPrecision = o.precision();
    o.precision(2);
    o.setf(std::ios::fixed, std::ios::floatfield);

    o << "Satellite " << t.name_ << ", el=" << el << " at " << pos;

    o.precision(oldPrecision);
    o.setf(oldFlags);
    return o;
}

/* End SATELITE.CPP *************************************************/
