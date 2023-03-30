/*
 * C S 2 P O L Y G . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    PhysCS2dPolygon

    Represents an polygon registered in a PhysConfigSpace2d
*/

#ifndef _PHYS_CS2POLYG_HPP
#define _PHYS_CS2POLYG_HPP

#include "base/base.hpp"
#include "stdlib/memory.hpp"
#include "mathex/mathex.hpp"
#include "mathex/abox2d.hpp"

#include "phys/cspace2.hpp"

// Forward refs
class MexPolygon2d;

// orthodox canonical (revoked)
class PhysCS2dPolygon
{
public:
    using PolygonId = PhysConfigSpace2d::PolygonId;

    using Flags = uint32;

    // ctor. TODO autoptr was const
    PhysCS2dPolygon(
        const PolygonId& id,
        std::unique_ptr<MexPolygon2d>& polygonAPtr,
        MATHEX_SCALAR height,
        Flags flags,
        const PhysConfigSpace2d::Longevity& longevity);

    // dtor.
    ~PhysCS2dPolygon();

    // Data access
    const PolygonId& id() const;
    const MexPolygon2d& polygon() const;
    MATHEX_SCALAR height() const;
    PhysConfigSpace2d::Longevity longevity() const;
    void longevity(PhysConfigSpace2d::Longevity newLongevity);
    const MexAlignedBox2d& boundary() const;
    void boundary(MexAlignedBox2d* pBox) const;

    bool isEnabled() const;
    void isEnabled(bool doEnable);

    //  Returns true iff the polygon isEnabled() and there is no
    //  match between the flags passed in and the flags already stored.
    bool isEnabled(Flags flags) const;

    Flags flags() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const PhysCS2dPolygon& t);

private:
    // Operation deliberately revoked
    PhysCS2dPolygon(const PhysCS2dPolygon&);
    PhysCS2dPolygon& operator=(const PhysCS2dPolygon&);
    bool operator==(const PhysCS2dPolygon&);

    // Data members
    PolygonId id_; // The id
    MexPolygon2d& polygon_; // The polygon
    MATHEX_SCALAR height_; // Highest z coordinate
    Flags flags_; // Allows polygons to be excluded for pathfinding purposes
    PhysConfigSpace2d::Longevity longevity_; // temporary/permanent type
    MexAlignedBox2d boundary_; // Aligned 2d bounding box
    bool enabled_ : 1; // True iff enabled for containment/intersections tests etc
};

#ifdef _INLINE
#include "internal/cs2polyg.ipp"
#endif

#endif

/* End CS2POLYG.HPP *************************************************/
