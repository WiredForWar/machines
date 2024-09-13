/*
 * A C T E X P L O . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "utility/linetok.hpp"
#include "mathex/transf3d.hpp"
#include "mathex/abox3d.hpp"
#include "world4d/generic.hpp"
#include "world4d/domain.hpp"
#include "sim/manager.hpp"
#include "machphys/objexplo.hpp"
#include "machlog/actexplo.hpp"
#include "machlog/plandoms.hpp"
#include "machlog/lineproj.hpp"

PER_DEFINE_PERSISTENT(MachLogExplosionAction);

MachLogExplosionAction::MachLogExplosionAction(SimCondition* pCondition, bool enabled)
    : SimAction(pCondition, enabled)
    , point_(MexPoint2d(0, 0))
    , range_(0)
    , damage_(0)
    , size_(0)
{
    TEST_INVARIANT;
}

// virtual
MachLogExplosionAction::~MachLogExplosionAction()
{
    TEST_INVARIANT;
}

void MachLogExplosionAction::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogExplosionAction& t)
{

    o << "MachLogExplosionAction " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogExplosionAction " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachLogExplosionAction::doAction()
{
    MexPoint3d location(point_.x(), point_.y(), MachLogPlanetDomains::terrainHeight(point_));
    MexTransform3d localTransform;
    W4dDomain* pDomain = MachLogPlanetDomains::pDomainPosition(location, 0, &localTransform);

    W4dEntity* pOwner = new W4dGeneric(pDomain, localTransform, W4dEntity::NOT_SOLID);
    pOwner->boundingVolume(MexAlignedBox3d(location, size_, size_, size_));
    // Construct and use an explosion
    MachPhysObjectExplosion exploder(pOwner);
    exploder.explode(SimManager::instance().currentTime());
    delete pOwner;
    MachLogLinearProjectile::genericCheckForDamage(location, range_, MachLogLinearProjectile::LINEAR_DAMAGE, damage_);
}

// static
std::unique_ptr<MachLogExplosionAction> MachLogExplosionAction::newFromParser(SimCondition* pCondition, bool enabled, UtlLineTokeniser* pParser)
{
    std::unique_ptr<MachLogExplosionAction> pResult(new MachLogExplosionAction(pCondition, enabled));
    for (std::size_t i = 0; i < pParser->tokens().size(); ++i)
    {
        const std::string& token = pParser->tokens()[i];
        if (token == "AT")
            pResult->point_
                = MexPoint2d(atof(pParser->tokens()[i + 1].c_str()), atof(pParser->tokens()[i + 2].c_str()));
        else if (token == "RANGE")
            pResult->range_ = atof(pParser->tokens()[i + 1].c_str());
        else if (token == "SIZE")
        {
            pResult->size_ = atof(pParser->tokens()[i + 1].c_str());
        }
        else if (token == "DAMAGE")
            pResult->damage_ = atol(pParser->tokens()[i + 1].c_str());
    }
    ASSERT(pResult->size_ > 0, "SIZE of an explosion MUST be specified\n");
    return pResult;
}

// virtual
void MachLogExplosionAction::doOutputOperator(std::ostream& o) const
{
    SimAction::doOutputOperator(o);
}

void perWrite(PerOstream& ostr, const MachLogExplosionAction& action)
{
    const SimAction& base1 = action;

    ostr << base1;
    ostr << action.point_;
    ostr << action.range_;
    ostr << action.size_;
    ostr << action.damage_;
}

void perRead(PerIstream& istr, MachLogExplosionAction& action)
{
    SimAction& base1 = action;

    istr >> base1;
    istr >> action.point_;
    istr >> action.range_;
    istr >> action.size_;
    istr >> action.damage_;
}

MachLogExplosionAction::MachLogExplosionAction(PerConstructor con)
    : SimAction(con)
{
}

/* End ACTREINF.CPP *************************************************/
