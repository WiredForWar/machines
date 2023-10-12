/*
 * E D I T D O M A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedDomainEditor

    A brief description of the class should go in here
*/

#ifndef _PLANETED_EDITDOMA_HPP
#define _PLANETED_EDITDOMA_HPP

#include "base/base.hpp"
#include "mathex/point2d.hpp"
#include "ctl/vector.hpp"
#include "ctl/pvector.hpp"
#include "planeted/editpoly.hpp"

class PedDomainEditor : public PedPolygonEditor
// Canonical form revoked
{
public:
    PedDomainEditor();
    ~PedDomainEditor() override;

    void CLASS_INVARIANT;

    void processInput(const DevButtonEvent&) override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    void displayKeyboardCtrls() override;

    void displayModeInfo() override;

    void writeCspFile(std::ofstream&) override;

    void readCspFile(const SysPathName&) override;
    // PRE( cspFileName.existsAsFile() );

    void validate() override;

    void processInsertVertex();
    void processDeleteVertex();

protected:
    PedPolygon* createPolygon(const PolyVerticies& verticies, MATHEX_SCALAR height, bool selected) const override;
    PedPolygon* createDefaultPolygon() const override;
    void processVertexRight() override;
    void processVertexLeft() override;
    void processVertexUp() override;
    void processVertexDown() override;
    void updatePolygon() override;

private:
    // Operations deliberately revoked
    PedDomainEditor(const PedDomainEditor&);
    PedDomainEditor& operator=(const PedDomainEditor&);
    bool operator==(const PedDomainEditor&);

    friend std::ostream& operator<<(std::ostream& o, const PedDomainEditor& t);

    void changeX(size_t fromX, size_t toX);
    void changeY(size_t fromY, size_t toY);
};

#endif

/* End EDITDOMA.HPP *************************************************/
