#pragma once

#include "ctl/NoBoundsVector.hpp"
#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "phys/Plans/MotionChunk.hpp"

// The recovery ignore list describes overlap now, not a future crossing of the
// current position. Keep the legacy short lookback, but no future occupancy.
inline PhysConfigSpace2d::ChunkIntersections machlogCurrentMotionOverlaps(
    PhysConfigSpace2d& space, const MexPoint2d& position, double clearance,
    PhysAbsoluteTime now, double minHeight, double maxHeight)
{
    PhysMotionChunk probe(position, position, clearance,
        PhysRampAcceleration(0.0, 400.0, -400.0, 0.01, 0.005, 0.005),
        now - 0.01, 0.0, minHeight, maxHeight);
    probe.expiryTime(now);
    PhysConfigSpace2d::ChunkIntersections intersections;
    intersections.reserve(3);
    space.findIntersections(probe, PhysConfigSpace2d::ObjectId::invalidId(), &intersections);
    return intersections;
}
