/*
 * P O R T A L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedPortal

    A brief description of the class should go in here
*/

#ifndef _PLANETED_PORTAL_HPP
#define _PLANETED_PORTAL_HPP

#include "base/base.hpp"

#include "mathex/point2d.hpp"

#include "planeted/polygon.hpp"

class ofstream;

class PedPortal : public PedPolygon
{
public:
    PedPortal(const PolyVerticies&, MATHEX_SCALAR height, bool selected);
    ~PedPortal() override;

    void CLASS_INVARIANT;

    void save(std::ofstream&) override;

    RenColour getWireColour() const override;

    size_t height() const override;

    size_t domainOne() const;
    size_t domainTwo() const;

    void setDomains(size_t domainOne, size_t domainTwo);

private:
    // Operations deliberately revoked
    PedPortal(const PedPortal&);
    PedPortal& operator=(const PedPortal&);
    bool operator==(const PedPortal&);

    friend ostream& operator<<(ostream& o, const PedPortal& t);

    // Data...
    size_t domainOne_;
    size_t domainTwo_;
};

#endif

/* End PORTAL.HPP ***************************************************/
