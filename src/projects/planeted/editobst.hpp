/*
 * O B S T E D I T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedObstacleEditor

    A brief description of the class should go in here
*/

#ifndef _PLANETED_OBSTEDIT_HPP
#define _PLANETED_OBSTEDIT_HPP

#include "base/base.hpp"
#include "mathex/point2d.hpp"
#include "ctl/vector.hpp"
#include "ctl/pvector.hpp"
#include "planeted/editpoly.hpp"
#include "planeted/obstacle.hpp"

class PedVerticalPolygonMarker;

class PedObstacleEditor : public PedPolygonEditor
// Canonical form revoked
{
public:
    PedObstacleEditor();
    ~PedObstacleEditor() override;

    void CLASS_INVARIANT;

    void initialise(W4dSceneManager*, MachLogPlanet*) override;
    // PRE( pPlanet_ != NULL );
    // PRE( pSceneManager_ != NULL );

    void processInput(const DevButtonEvent&) override;
    // PRE( pSceneManager_ != NULL );
    // PRE( pPlanet_ != NULL );

    void displayKeyboardCtrls() override;

    void displayModeInfo() override;

    void readCspFile(const SysPathName&) override;
    // PRE( cspFileName.existsAsFile() );

    void validate() override;

protected:
    PedPolygon* createPolygon(const PolyVerticies& verticies, MATHEX_SCALAR height, bool selected) const override;
    PedPolygon* createDefaultPolygon() const override;
    void updatePolygon() override;
    void processVertexRight() override;
    void processVertexLeft() override;
    void processVertexUp() override;
    void processVertexDown() override;
    void processInsertVertex();
    void processDeleteVertex();
    void processDisplayVerticalPolygons(bool displayPolygons);
    void processComputeVerticalPolygons();
    void highlightAllObstacles();
    void processSelectPolygon();
    void processChangeObstacleType();

private:
    // Operations deliberately revoked
    PedObstacleEditor(const PedObstacleEditor&);
    PedObstacleEditor& operator=(const PedObstacleEditor&);
    bool operator==(const PedObstacleEditor&);

    friend std::ostream& operator<<(std::ostream& o, const PedObstacleEditor& t);

    using VerticalPolygons = ctl_pvector<PedVerticalPolygonMarker>;

    VerticalPolygons verticalPolygons_;
    bool displayVerticalPolygons_;
};
#endif

/* End OBSTEDIT.HPP *************************************************/
