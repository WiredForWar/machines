/*
 * E D I T P O R T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedPortalEditor

    A brief description of the class should go in here
*/

#ifndef _PLANETED_EDITPORT_HPP
#define _PLANETED_EDITPORT_HPP

#include "base/base.hpp"
#include "mathex/point2d.hpp"
#include "ctl/vector.hpp"
#include "ctl/pvector.hpp"
#include "planeted/editpoly.hpp"

class PedDomainEditor;

class PedPortalEditor : public PedPolygonEditor
// Canonical form revoked
{
public:
    PedPortalEditor();
    ~PedPortalEditor() override;

    void CLASS_INVARIANT;

    void processInput(const DevButtonEvent&) override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    void displayKeyboardCtrls() override;

    void displayModeInfo() override;

    void readCspFile(const SysPathName&) override;
    // PRE( cspFileName.existsAsFile() );

    void writeCspFile(std::ofstream&) override;

    void validate() override;

    void domainEditor(PedDomainEditor*);

protected:
    PedPolygon* createPolygon(const PolyVerticies& verticies, MATHEX_SCALAR height, bool selected) const override;
    PedPolygon* createDefaultPolygon() const override;
    void processVertexRight() override;
    void processVertexLeft() override;
    void processVertexUp() override;
    void processVertexDown() override;
    void updatePolygon() override;

private:
    void processCreatePortalsFromDomains();

    // Operations deliberately revoked
    PedPortalEditor(const PedPortalEditor&);
    PedPortalEditor& operator=(const PedPortalEditor&);
    bool operator==(const PedPortalEditor&);

    friend std::ostream& operator<<(std::ostream& o, const PedPortalEditor& t);

    // Data...
    PedDomainEditor* pDomainEditor_;
};

#endif

/* End EDITPORT.HPP *************************************************/
