#include "machlog/Actors/CurrentMotionOverlap.hpp"

#include <gtest/gtest.h>

namespace
{
void add(PhysConfigSpace2d& space, std::size_t actor, const PhysMotionChunk& chunk)
{
    space.add(chunk, PhysConfigSpace2d::ObjectId(actor));
}

bool includes(const PhysConfigSpace2d::ChunkIntersections& hits, std::size_t actor)
{
    for (const PhysConfigSpace2d::ChunkIntersectionData& hit : hits)
        if (hit.collisionObjectId_.asScalar() == actor)
            return true;
    return false;
}
}

TEST(CurrentMotionOverlapTests, FutureCrossingCannotHideAnActorsReservations)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100),
        PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    // This peer passes the query position five seconds after the query time.
    const PhysMotionChunk peer(MexPoint2d(30, 50), MexPoint2d(70, 50), 1,
        PhysRampAcceleration(2, 1, -1, 20, 0, 0), 5, 0, 0, 10);
    add(space, 1, peer);
    EXPECT_FALSE(includes(machlogCurrentMotionOverlaps(space, MexPoint2d(50, 50), 1, 10, 0, 10), 1));
    EXPECT_TRUE(includes(machlogCurrentMotionOverlaps(space, MexPoint2d(50, 50), 1, 15, 0, 10), 1));
    EXPECT_FALSE(includes(machlogCurrentMotionOverlaps(space, MexPoint2d(50, 50), 1, 20, 0, 10), 1));
}

TEST(CurrentMotionOverlapTests, FutureCreationAndFinishedTravelAreTimeFiltered)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100),
        PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    const PhysRampAcceleration resting(0, 1, -1, .01, 0, 0);
    add(space, 1, PhysMotionChunk(MexPoint2d(50, 50), MexPoint2d(50, 50), 1,
        resting, 12, 0, 0, 10));
    PhysMotionChunk expired(MexPoint2d(50, 50), MexPoint2d(50, 50), 1,
        resting, 0, 0, 0, 10);
    expired.expiryTime(9);
    add(space, 2, expired);
    add(space, 3, PhysMotionChunk(MexPoint2d(50, 50), MexPoint2d(50, 50), 1,
        resting, 0, 0, 20, 30));
    add(space, 4, PhysMotionChunk(MexPoint2d(50, 50), MexPoint2d(50, 50), 1,
        resting, 0, 0, 0, 10));
    const PhysConfigSpace2d::ChunkIntersections hits{
        machlogCurrentMotionOverlaps(space, MexPoint2d(50, 50), 1, 10, 0, 10)};
    EXPECT_FALSE(includes(hits, 1));
    EXPECT_FALSE(includes(hits, 2));
    EXPECT_FALSE(includes(hits, 3));
    EXPECT_TRUE(includes(hits, 4));
}

TEST(CurrentMotionOverlapTests, FutureOnlyPeerStillBlocksAConflictingNewReservation)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100),
        PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    add(space, 1, PhysMotionChunk(MexPoint2d(30, 50), MexPoint2d(70, 50), 1,
        PhysRampAcceleration(2, 1, -1, 20, 0, 0), 5, 0, 0, 10));
    const PhysConfigSpace2d::ChunkIntersections overlaps{
        machlogCurrentMotionOverlaps(space, MexPoint2d(50, 50), 1, 10, 0, 10)};
    PhysConfigSpace2d::ObjectIds ignored;
    ignored.push_back(PhysConfigSpace2d::ObjectId(2));
    for (const PhysConfigSpace2d::ChunkIntersectionData& hit : overlaps)
        ignored.push_back(hit.collisionObjectId_);

    // We are clear now, but a five-second turn at this point would obstruct
    // the other machine's already accepted crossing. It must remain checked.
    const PhysMotionChunk candidate(MexPoint2d(50, 50), MexPoint2d(50, 70), 1,
        PhysRampAcceleration(2, 1, -1, 10, 0, 0), 10, 5, 0, 10);
    PhysConfigSpace2d::MotionChunkId id;
    PhysConfigSpace2d::ObjectId blocker;
    PhysAbsoluteTime collision{};
    EXPECT_FALSE(space.add(candidate, PhysConfigSpace2d::ObjectId(2), ignored,
        &id, &blocker, &collision));
    EXPECT_EQ(blocker.asScalar(), 1u);
    EXPECT_GE(collision, 10);
    EXPECT_LE(collision, 15);
}
