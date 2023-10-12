/*
 * E D I T C A M R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    PedCameraEditor

    Camera editor mode. Used for setting the initial camera positions for each race.
*/

#ifndef _PLANETED_EDITCAMR_HPP
#define _PLANETED_EDITCAMR_HPP

#include "base/base.hpp"
#include "device/butevent.hpp"
#include "render/colour.hpp"
#include "machphys/machphys.hpp"
#include "planeted/editmode.hpp"

// class DevButtonEvent;
class SysPathName;
class ofstream;
class PedCameraMarker;

class PedCameraEditor : public PedEditorMode
// Canonical form revoked
{
public:
    PedCameraEditor();
    ~PedCameraEditor() override;

    void CLASS_INVARIANT;

    void initialise(W4dSceneManager*, MachLogPlanet*) override;
    // PRE( pPlanet != NULL );
    // PRE( pSceneManager != NULL );

    void processInput(const DevButtonEvent&) override;

    void changingMode() override;

    void preRenderUpdate() override;

    void displayKeyboardCtrls() override;

    void displayModeInfo() override;

    void readScnFile(PedScenarioFile&) override;
    // PRE( cspFileName.existsAsFile() );

    void writeScnFile(PedScenarioFile&) override;

    void activateMode() override;

private:
    // Operations deliberately revoked
    PedCameraEditor(const PedCameraEditor&);
    PedCameraEditor& operator=(const PedCameraEditor&);
    bool operator==(const PedCameraEditor&);

    friend std::ostream& operator<<(std::ostream& o, const PedCameraEditor& t);

    void processDrop();

    PedCameraMarker* processChangeRace(PedCameraMarker*);

    void setCameraToMarker(const PedCameraMarker&);

    void setMarkerToCamera(PedCameraMarker**, MachPhys::Race);

    PedCameraMarker* setMarkerForRace(MachPhys::Race);

    // Data

    PedCameraMarker* pRedMarker_;
    PedCameraMarker* pBlueMarker_;
    PedCameraMarker* pGreenMarker_;
    PedCameraMarker* pYellowMarker_;
    PedCameraMarker* pCurrentMarker_;
    MachPhys::Race race_;
};

#endif

/* End EDITCAMR.HPP *************************************************/
