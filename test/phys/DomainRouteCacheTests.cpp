#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include "mathex/AlignedBox2d.hpp"
#include "mathex/ConvexPolygon2d.hpp"
#include "ctl/NoBoundsVector.hpp"

#include <gtest/gtest.h>

namespace
{
void addFloor(PhysConfigSpace2d& space, bool connected)
{
    const PhysConfigSpace2d::DomainId left{space.addDomain(MexAlignedBox2d(0, 0, 50, 100))};
    const PhysConfigSpace2d::DomainId right{space.addDomain(MexAlignedBox2d(50, 0, 100, 100))};
    if (connected)
        space.addPortal(left, right, MexPoint2d(50, 0), MexPoint2d(50, 100));
}

bool route(PhysConfigSpace2d& space, double y, PhysConfigSpace2d::PortalPoints* points,
    unsigned* updates = nullptr, double budget = .01)
{
    const PhysConfigSpace2d::DomainFindPathId query{space.addDomainFindPath(
        MexPoint2d(10, y), MexPoint2d(90, y), 1, 0, PhysPathFindingPriority(1))};
    for (unsigned turn{}; turn < 1000 && !space.isDomainFindPathDone(query); ++turn)
    {
        space.updateDomainFindPath(query, budget);
        if (updates)
            ++*updates;
    }
    EXPECT_TRUE(space.isDomainFindPathDone(query));
    return space.domainFindPath(query, points);
}

TEST(DomainRouteCacheTests, UnchangedSpaceRetainsWarmRouteReuse)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(space, true);
    PhysConfigSpace2d::PortalPoints cold, warm;
    unsigned coldUpdates{}, warmUpdates{};
    ASSERT_TRUE(route(space, 5, &cold, &coldUpdates, 0));
    ASSERT_TRUE(route(space, 5, &warm, &warmUpdates, 0));
    EXPECT_EQ(cold, warm);
    EXPECT_LT(warmUpdates, coldUpdates);
}
}

TEST(DomainRouteCacheTests, ConnectedSpaceCannotSupplyPortalsToADisconnectedSpace)
{
    PhysConfigSpace2d connected(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    PhysConfigSpace2d disconnected(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(connected, true);
    addFloor(disconnected, false);
    PhysConfigSpace2d::PortalPoints points;
    ASSERT_TRUE(route(connected, 50, &points));
    ASSERT_FALSE(points.empty());
    points.clear();
    EXPECT_FALSE(route(disconnected, 50, &points));
    EXPECT_TRUE(points.empty());
    EXPECT_TRUE(route(connected, 50, &points));
    EXPECT_FALSE(points.empty());
}

TEST(DomainRouteCacheTests, ReplacedPortalCannotReuseDistancesAlongTheOldOpening)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(space, true);
    PhysConfigSpace2d::PortalPoints points;
    ASSERT_TRUE(route(space, 20, &points));
    ASSERT_FALSE(points.empty());
    space.removePortal(points.front().first);
    space.addPortal(PhysConfigSpace2d::DomainId(0), PhysConfigSpace2d::DomainId(1), MexPoint2d(50, 70), MexPoint2d(50, 72.5));
    points.clear();
    ASSERT_TRUE(route(space, 20, &points));
    ASSERT_FALSE(points.empty());
    for (const PhysConfigSpace2d::PortalPoint& point : points)
    {
        EXPECT_GE(point.second, 0);
        EXPECT_LE(point.second, space.portalLength(point.first));
    }
}

TEST(DomainRouteCacheTests, PermanentWallInvalidatesAWarmPortalRoute)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(space, true);
    PhysConfigSpace2d::PortalPoints points;
    ASSERT_TRUE(route(space, 65, &points));
    ASSERT_FALSE(points.empty());
    std::unique_ptr<MexPolygon2d> wall{new MexConvexPolygon2d(MexAlignedBox2d(49, 0, 51, 100))};
    space.add(wall, 3, 0, PhysConfigSpace2d::PERMANENT);
    points.clear();
    EXPECT_FALSE(route(space, 65, &points));
    EXPECT_TRUE(points.empty());
}

TEST(DomainRouteCacheTests, RemovedPortalInvalidatesAWarmDomainRoute)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(space, true);
    PhysConfigSpace2d::PortalPoints points;
    ASSERT_TRUE(route(space, 80, &points));
    ASSERT_FALSE(points.empty());
    const PhysConfigSpace2d::PortalId portal{points.front().first};
    space.removePortal(portal);
    points.clear();
    EXPECT_FALSE(route(space, 80, &points));
    EXPECT_TRUE(points.empty());
}

TEST(DomainRouteCacheTests, RepeatedDisconnectedQueriesCannotBecomeCachedSuccesses)
{
    PhysConfigSpace2d space(MexPoint2d(0, 0), MexPoint2d(100, 100), PhysConfigSpace2d::SUBTRACTIVE, 1, 1, 3, 3);
    addFloor(space, false);
    PhysConfigSpace2d::PortalPoints points;
    ASSERT_FALSE(route(space, 50, &points));
    EXPECT_TRUE(points.empty());
    EXPECT_FALSE(route(space, 50, &points));
    EXPECT_TRUE(points.empty());
}
