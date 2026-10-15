/*
 * C S 2 D M C A C . H P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

/*
    PhysCS2dDomainFindPathCache

    A brief description of the class should go in here
*/

#ifndef _PHYS_CS2DMCAC_HPP
#define _PHYS_CS2DMCAC_HPP

#include "base/base.hpp"
#include "mathex/Point2d.hpp"
#include "ctl/Utility.hpp"
#include "ctl/Vector.hpp"
#include "ctl/NoBoundsVector.hpp"
#include "phys/ConfigSpace/ConfigSpace2d.hpp"
#include <cstdint>

class PhysCS2dDomainFindPathCache
// Canonical form revoked
{
public:
    ~PhysCS2dDomainFindPathCache();

    void clear();
    std::uint64_t generation() const;

    using PortalPoints = PhysConfigSpace2d::PortalPoints;

    using ObstacleFlags = PhysConfigSpace2d::ObstacleFlags;

    bool findPath(
        PhysConfigSpace2d::DomainId startDomainId,
        const MexPoint2d& startPoint,
        PhysConfigSpace2d::DomainId endDomainId,
        const MexPoint2d& endPoint,
        MATHEX_SCALAR clearance,
        ObstacleFlags flags,
        PortalPoints* pPath) const;

    void addPath(
        PhysConfigSpace2d::DomainId startDomainId,
        const MexPoint2d& startPoint,
        PhysConfigSpace2d::DomainId endDomainId,
        const MexPoint2d& endPoint,
        MATHEX_SCALAR clearance,
        ObstacleFlags flags,
        const PortalPoints& path,
        std::uint64_t generation);

    void CLASS_INVARIANT;

private:
    friend class PhysCS2dImpl;
    friend std::ostream& operator<<(std::ostream& o, const PhysCS2dDomainFindPathCache& t);

    PhysCS2dDomainFindPathCache(const PhysCS2dDomainFindPathCache&);
    PhysCS2dDomainFindPathCache& operator=(const PhysCS2dDomainFindPathCache&);

    PhysCS2dDomainFindPathCache();

    //  To allow template instantiation
    friend void PhysDummyFunction2();

    CTL_PAIR(Point, PhysConfigSpace2d::DomainId, domainId, MexPoint2d, point);

    CTL_PAIR(SearchData, MATHEX_SCALAR, clearance, ObstacleFlags, flags);

    CTL_QUAD(PathData, Point, startPoint, Point, endPoint, SearchData, searchData, PortalPoints, path);

    using Cache = ctl_vector<PathData>;

    Cache cache_{};
    uint oldestValue_{};
    const uint maxCacheValues_;

    uint succeed_{};
    uint fail_{};
    std::uint64_t generation_{};
};

#endif

/* End CS2DMCAC.HPP *************************************************/
