#include <gtest/gtest.h>

#include "mathex/Line3d.hpp"
#include "mathex/Point3d.hpp"

namespace
{

constexpr MATHEX_SCALAR tolerance = 1e-9;

// Along the x axis from the origin, which is the shape of a cursor ray: the
// distance reported back is then just the x coordinate of the closest point.
MexLine3d alongX(MATHEX_SCALAR length)
{
    return MexLine3d(MexPoint3d(0, 0, 0), MexPoint3d(length, 0, 0));
}

} // namespace

TEST(MexLine3dTests, CrossingSegmentsMeet)
{
    MATHEX_SCALAR along = -1;
    const MexLine3d across(MexPoint3d(4, -1, 0), MexPoint3d(4, 1, 0));

    EXPECT_NEAR(0.0, alongX(10).closestApproach(across, &along), tolerance);
    EXPECT_NEAR(4.0, along, tolerance);
}

TEST(MexLine3dTests, PassingSegmentsAreApartByTheGap)
{
    MATHEX_SCALAR along = -1;
    const MexLine3d across(MexPoint3d(4, -1, 3), MexPoint3d(4, 1, 3));

    EXPECT_NEAR(3.0, alongX(10).closestApproach(across, &along), tolerance);
    EXPECT_NEAR(4.0, along, tolerance);
}

TEST(MexLine3dTests, TheAnswerStaysOnBothSegments)
{
    // The infinite lines cross at x = 40, well past the end of a segment that
    // stops at 10, so the closest points are the near ends of each.
    MATHEX_SCALAR along = -1;
    const MexLine3d across(MexPoint3d(40, -1, 0), MexPoint3d(40, 1, 0));

    EXPECT_NEAR(30.0, alongX(10).closestApproach(across, &along), tolerance);
    EXPECT_NEAR(10.0, along, tolerance);
}

TEST(MexLine3dTests, ParallelSegmentsAreApartBySeparation)
{
    MATHEX_SCALAR along = -1;
    const MexLine3d beside(MexPoint3d(0, 2, 0), MexPoint3d(10, 2, 0));

    EXPECT_NEAR(2.0, alongX(10).closestApproach(beside, &along), tolerance);
}

TEST(MexLine3dTests, ParallelSegmentsThatDoNotOverlapMeetAtTheirEnds)
{
    MATHEX_SCALAR along = -1;
    const MexLine3d beyond(MexPoint3d(13, 4, 0), MexPoint3d(20, 4, 0));

    EXPECT_NEAR(5.0, alongX(10).closestApproach(beyond, &along), tolerance);
    EXPECT_NEAR(10.0, along, tolerance);
}

TEST(MexLine3dTests, ASegmentOfNoLengthIsAPoint)
{
    MATHEX_SCALAR along = -1;
    const MexLine3d point(MexPoint3d(4, 3, 0), MexPoint3d(4, 3, 0));

    EXPECT_NEAR(3.0, alongX(10).closestApproach(point, &along), tolerance);
    EXPECT_NEAR(4.0, along, tolerance);
}

TEST(MexLine3dTests, SkewSegmentsAreApartByTheirCommonPerpendicular)
{
    // One along x at z = 0, one along y at z = 5: they never meet, and the
    // shortest link between them is the 5 units of height.
    MATHEX_SCALAR along = -1;
    const MexLine3d overhead(MexPoint3d(6, -4, 5), MexPoint3d(6, 4, 5));

    EXPECT_NEAR(5.0, alongX(10).closestApproach(overhead, &along), tolerance);
    EXPECT_NEAR(6.0, along, tolerance);
}
