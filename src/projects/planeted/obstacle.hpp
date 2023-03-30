/*
 * O B S T A C L E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedObstacle

    A brief description of the class should go in here
*/

#ifndef _PLANETED_OBSTACLE_HPP
#define _PLANETED_OBSTACLE_HPP

#include "base/base.hpp"

#include "mathex/point2d.hpp"
#include "stdlib/string.hpp"
#include "planeted/polygon.hpp"

class PedObstacle : public PedPolygon
{
public:
    enum CspType
    {
        NORMAL,
        LOW,
        WATER
    };

    PedObstacle(const PolyVerticies&, MATHEX_SCALAR height, bool selected, CspType cspType);
    ~PedObstacle() override;

    void CLASS_INVARIANT;

    void save(std::ofstream&) override;

    RenColour getWireColour() const override;

    void select(bool selected) override;

    CspType cspType() const { return cspType_; }

    void cspType(CspType cspType) { cspType_ = cspType; }

    static CspType highlightedType() { return highlightedType_; }

    static void highlightedType(CspType cspType) { highlightedType_ = cspType; }

private:
    // Operations deliberately revoked
    PedObstacle(const PedObstacle&);
    PedObstacle& operator=(const PedObstacle&);
    bool operator==(const PedObstacle&);

    friend std::ostream& operator<<(std::ostream& o, const PedObstacle& t);

    CspType cspType_;
    static CspType highlightedType_;
};

std::ostream& operator<<(std::ostream& o, PedObstacle::CspType cspType);

#endif

/* End OBSTACLE.HPP *************************************************/
