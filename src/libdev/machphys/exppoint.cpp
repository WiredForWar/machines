/*
 * E X P P O I N T . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/cexpdata.hpp"

MachPhysConstructionExplosionData::ExplosionPoint::ExplosionPoint()
{
    ASSERT_FAIL("");
}

MachPhysConstructionExplosionData::ExplosionPoint::ExplosionPoint(
    const MexPoint3d& position,
    MATHEX_SCALAR maxFireballOffset,
    size_t minFireballs,
    size_t maxFireballs,
    MATHEX_SCALAR minTimeFactor,
    MATHEX_SCALAR maxTimeFactor,
    MATHEX_SCALAR size,
    MATHEX_SCALAR depthOffset)
    : position_(position)
    , maxFireballOffset_(maxFireballOffset)
    , minFireballs_(minFireballs)
    , maxFireballs_(maxFireballs)
    , minTimeFactor_(minTimeFactor)
    , maxTimeFactor_(maxTimeFactor)
    , size_(size)
    , depthOffset_(depthOffset)
{
    PRE_INFO(minTimeFactor);
    PRE_INFO(maxTimeFactor);
    PRE(0.0 <= minTimeFactor and minTimeFactor <= 1.0);
    PRE(0.0 <= maxTimeFactor and maxTimeFactor <= 1.0);

    TEST_INVARIANT;
}

MachPhysConstructionExplosionData::ExplosionPoint::~ExplosionPoint()
{
    TEST_INVARIANT;
}

const MexPoint3d& MachPhysConstructionExplosionData::ExplosionPoint::position() const
{
    return position_;
}

MATHEX_SCALAR MachPhysConstructionExplosionData::ExplosionPoint::maxFireballOffset() const
{
    return maxFireballOffset_;
}

size_t MachPhysConstructionExplosionData::ExplosionPoint::minFireballs() const
{
    return minFireballs_;
}

size_t MachPhysConstructionExplosionData::ExplosionPoint::maxFireballs() const
{
    return maxFireballs_;
}

MATHEX_SCALAR MachPhysConstructionExplosionData::ExplosionPoint::minTimeFactor() const
{
    MATHEX_SCALAR result = minTimeFactor_;

    POST_INFO(minTimeFactor_);
    POST(0.0 <= result and result <= 1.0);

    return result;
}

MATHEX_SCALAR MachPhysConstructionExplosionData::ExplosionPoint::maxTimeFactor() const
{
    MATHEX_SCALAR result = maxTimeFactor_;

    POST_INFO(maxTimeFactor_);
    POST(0.0 <= result and result <= 1.0);

    return result;
}

MATHEX_SCALAR MachPhysConstructionExplosionData::ExplosionPoint::size() const
{
    return size_;
}

MATHEX_SCALAR MachPhysConstructionExplosionData::ExplosionPoint::depthOffset() const
{
    return depthOffset_;
}

void MachPhysConstructionExplosionData::ExplosionPoint::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysConstructionExplosionData::ExplosionPoint& t)
{

    o << "ExplosionPoint " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "ExplosionPoint " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End EXPPOINT.CPP *************************************************/
