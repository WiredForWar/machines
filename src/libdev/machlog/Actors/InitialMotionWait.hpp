#pragma once

#include "phys/Plans/MotionChunk.hpp"

// The first accepted motion can start later than its admission. Its body waits
// at the start point in that interval; keep the actual travel timing unchanged.
inline PhysMotionChunk machlogCoverInitialMotionWait(const PhysMotionChunk& motion, PhysAbsoluteTime now)
{
    if (now >= motion.createTime())
        return motion;
    PhysMotionChunk covered(motion.startPoint(), motion.endPoint(), motion.clearance(),
        motion.motionProfile(), now, motion.motionTimeOffset() + (motion.createTime() - now),
        motion.minHeight(), motion.maxHeight());
    if (motion.hasExpiryTime())
        covered.expiryTime(motion.expiryTime());
    return covered;
}
