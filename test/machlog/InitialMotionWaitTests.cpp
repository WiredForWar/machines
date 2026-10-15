#include "machlog/Actors/InitialMotionWait.hpp"
#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "ctl/NoBoundsVector.hpp"

#include <gtest/gtest.h>

TEST(InitialMotionWaitTests, WaitingBodyBlocksAnAlreadyAcceptedCrossing)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100),
        PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    const PhysMotionChunk peer(MexPoint2d(30, 50), MexPoint2d(70, 50), 1,
        PhysRampAcceleration(2, 1, -1, 20, 0, 0), 5, 0, 0, 10);
    space.add(peer, PhysConfigSpace2d::ObjectId(1));
    const PhysMotionChunk original(MexPoint2d(50, 50), MexPoint2d(50, 69.6), 1,
        PhysRampAcceleration(0, 10, -10, 10, .2, .2), 20, 2, 0, 10);
    PhysAbsoluteTime collision{};
    EXPECT_FALSE(original.intersects(peer, &collision));
    const PhysMotionChunk covered{machlogCoverInitialMotionWait(original, 10)};
    EXPECT_TRUE(covered.intersects(peer, &collision));
    EXPECT_GE(collision, 10);
    EXPECT_LE(collision, 20);
    PhysConfigSpace2d::MotionChunkId id;
    PhysConfigSpace2d::ObjectId blocker;
    EXPECT_FALSE(space.add(covered, PhysConfigSpace2d::ObjectId(2),
        &id, &blocker, &collision));
    EXPECT_EQ(blocker.asScalar(), 1u);
}

TEST(InitialMotionWaitTests, TravelCurveAndAbsoluteExpiryStayUnchanged)
{
    PhysMotionChunk original(MexPoint2d(50, 50), MexPoint2d(50, 69.6), 1,
        PhysRampAcceleration(0, 10, -10, 10, .2, .2), 20, 2, 0, 10);
    original.expiryTime(33);
    const PhysMotionChunk covered{machlogCoverInitialMotionWait(original, 10)};
    EXPECT_EQ(covered.createTime(), 10);
    EXPECT_EQ(covered.motionTimeOffset(), 12);
    EXPECT_EQ(covered.createTime() + covered.motionTimeOffset(),
        original.createTime() + original.motionTimeOffset());
    EXPECT_EQ(covered.motionProfile().totalTime(), original.motionProfile().totalTime());
    EXPECT_EQ(covered.motionProfile().totalDistance(), original.motionProfile().totalDistance());
    EXPECT_EQ(covered.motionProfile().firstAccelerationTime(), original.motionProfile().firstAccelerationTime());
    EXPECT_EQ(covered.motionProfile().secondAccelerationTime(), original.motionProfile().secondAccelerationTime());
    EXPECT_EQ(covered.clearance(), original.clearance());
    EXPECT_EQ(covered.expiryTime(), original.expiryTime());
    EXPECT_EQ(covered.startPoint(), original.startPoint());
    EXPECT_EQ(covered.endPoint(), original.endPoint());
    EXPECT_EQ(original.createTime(), 20);
}

TEST(InitialMotionWaitTests, CurrentOrPastCreationDoesNotMoveForward)
{
    const PhysMotionChunk original(MexPoint2d(50, 50), MexPoint2d(50, 69.6), 1,
        PhysRampAcceleration(0, 10, -10, 10, .2, .2), 20, 2, 0, 10);
    for (const double now : {20.0, 25.0,})
    {
        const PhysMotionChunk covered{machlogCoverInitialMotionWait(original, now)};
        EXPECT_EQ(covered.createTime(), original.createTime());
        EXPECT_EQ(covered.motionTimeOffset(), original.motionTimeOffset());
    }
}
