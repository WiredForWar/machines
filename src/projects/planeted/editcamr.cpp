/*
 * E D I T C A M R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "planeted/editcamr.hpp"
#include <fstream>
#include "device/butevent.hpp"
#include "system/pathname.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "mathex/eulerang.hpp"
#include "mathex/degrees.hpp"
#include "render/colour.hpp"
#include "machlog/World/Camera.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "planeted/cameras.hpp"
#include "planeted/camrmrkr.hpp"
#include "planeted/editactor.hpp"
#include "planeted/race.hpp"
#include "planeted/scenfile.hpp"

PedCameraEditor::PedCameraEditor()
    : pMarkers_{nullptr}
    , race_(MachPhys::RED)
{

    TEST_INVARIANT;
}

// virtual
void PedCameraEditor::initialise(W4dSceneManager* pScene, MachLogPlanet* pPlanet)
{
    PedEditorMode::initialise(pScene, pPlanet);

    // Set all camera markers to be at the current camera position
    setMarkerToCamera(&pMarkers_[MachPhys::RED], MachPhys::RED);
    setMarkerToCamera(&pMarkers_[MachPhys::BLUE], MachPhys::BLUE);
    setMarkerToCamera(&pMarkers_[MachPhys::GREEN], MachPhys::GREEN);
    setMarkerToCamera(&pMarkers_[MachPhys::YELLOW], MachPhys::YELLOW);
}

PedCameraEditor::~PedCameraEditor()
{
    TEST_INVARIANT;
}

void PedCameraEditor::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void PedCameraEditor::processInput(const DevButtonEvent& devButtonEvent)
{
    // Process keyboard/mouse events

    if (devButtonEvent.action() == DevButtonEvent::PRESS && active_)
    {
        if (devButtonEvent.scanCode() == Device::KeyCode::KEY_D)
        {
            processDrop();
        }
        else if (devButtonEvent.scanCode() == Device::KeyCode::KEY_C)
        {
            processChangeRace();
            setCameraToMarker(*pMarkers_[race_]);
        }
    }
}

// virtual
void PedCameraEditor::changingMode()
{
    PedEditorMode::changingMode();
}

// virtual
void PedCameraEditor::preRenderUpdate()
{
}

// virtual
void PedCameraEditor::displayKeyboardCtrls()
{
    PRE(pSceneManager_ != nullptr);

    pSceneManager_->out() << "D : mark camera position for this race" << std::endl;
    pSceneManager_->out() << "C: show next race's camera position" << std::endl;
}

// virtual
void PedCameraEditor::displayModeInfo()
{
    PRE(pSceneManager_ != nullptr);

    TEST_INVARIANT;

    pSceneManager_->out() << "MODE : Camera Edit" << std::endl;
    pSceneManager_->out() << "Race: " << race_ << std::endl;
}

// virtual
void PedCameraEditor::readScnFile(PedScenarioFile& scenarioFile)
{
    MachPhys::Race race = MachPhys::RED;
    MexPoint3d origin;

    for (uint nRace = 0; nRace < PedRace::nRaces(); ++nRace)
    {
        const PedScenarioFile::Camera& scnCamera = scenarioFile.camera(race);

        // Only reinitialise position if new position is not the origin
        if (!(scnCamera.transform.position() == origin))
        {
            W4dDomain* pDomain = MachLogPlanetDomains::pDomainAt(scnCamera.transform.position());

            delete pMarkers_[race];

            // Calculate local transform from global transform and parent global transform
            MexTransform3d localResultTransform;
            const MexTransform3d parentGlobalTransform = pDomain->globalTransform();
            parentGlobalTransform.transformInverse(scnCamera.transform, &localResultTransform);

            RenColour markerColour = PedRace::colour(race);
            // Create new marker at local transform calculated above
            pMarkers_[race] = new PedCameraMarker(pDomain, localResultTransform, scnCamera.type, markerColour);
            pMarkers_[race]->draw();
            race = PedRace::next(race);
        }
    }
}

// virtual
void PedCameraEditor::writeScnFile(PedScenarioFile& scenarioFile)
{
    MachPhys::Race currentRace = MachPhys::RED;
    for (uint nRace = 0; nRace < PedRace::nRaces(); ++nRace)
    {
        PedCameraMarker* pCurrentMarker = pMarkers_[currentRace];
        PedScenarioFile::Camera& scnCamera = scenarioFile.camera(currentRace);
        scnCamera.transform = pCurrentMarker->globalTransform();
        scnCamera.type = pCurrentMarker->cameraType();
        currentRace = PedRace::next(currentRace);
    }
}

// virtual
void PedCameraEditor::activateMode()
{
    PedEditorMode::activateMode();
    setCameraToMarker(*pMarkers_[race_]);
}

std::ostream& operator<<(std::ostream& o, const PedCameraEditor& t)
{

    o << "PedCameraEditor " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "PedCameraEditor " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void PedCameraEditor::processDrop()
{
    setMarkerToCamera(&pMarkers_[race_], race_);
}

void PedCameraEditor::processChangeRace()
{
    // Set pCurrentMarker to the next race - note race must rotate in the same order
    // as in PedRace::next(...)

    switch(race_)
    {
        case MachPhys::RED:
            race_ = MachPhys::BLUE;
            break;
        case MachPhys::BLUE:
            race_ = MachPhys::GREEN;
            break;
        case MachPhys::GREEN:
            race_ = MachPhys::YELLOW;
            break;
        case MachPhys::YELLOW:
            race_ = MachPhys::RED;
            break;
    }
    return; // ok to do this as it is assigned to point to a valid member marker
}

void PedCameraEditor::setCameraToMarker(const PedCameraMarker& marker)
{
    // Set camera postion to be that of current camera marker
    // Set camera view mode
    switch (marker.cameraType())
    {
        case (MachLogCamera::GROUND):
            MachCameras::instance().switchToGroundView();
            break;
        case (MachLogCamera::ZENITH):
            MachCameras::instance().switchToZenithView();
            break;
    }
    const MexTransform3d& newCameraTransform = marker.globalTransform();
    MachLogCamera* pCurrentCamera = MachCameras::instance().currentCamera();
    W4dDomain* pDomain = pCurrentCamera->containingDomain();

    // Get the camera's current global 2d position, and hence the domain it should be in
    W4dDomain* pCorrectDomain = MachLogPlanetDomains::pDomainAt(newCameraTransform.position());

    // If not attached to the right domain, attach it now
    if (pDomain != pCorrectDomain)
    {
        pCurrentCamera->attachTo(pCorrectDomain);
        pDomain = pCorrectDomain;
    }
    // Set camera postion to be that of current camera marker
    pCurrentCamera->globalTransform(newCameraTransform);
}

void PedCameraEditor::setMarkerToCamera(PedCameraMarker** ppCameraMarker, MachPhys::Race race)
{
    // Create a camera marker at the current camera position

    // Check camera mode is valid
    MachLogCamera::Type cameraType = MachCameras::instance().currentCameraType();
    if ((cameraType == MachLogCamera::GROUND) || (cameraType == MachLogCamera::ZENITH))
    {
        // Get the local transform for the camera
        MachLogCamera* pCurrentCamera = MachCameras::instance().currentCamera();
        const MexTransform3d& cameraLocalTransform = pCurrentCamera->localTransform();

        // Get the camera's parent entity
        W4dEntity* pParent = pCurrentCamera->pParent();

        // Use the transform and parent to attach a camera marker in the correct position
        RenColour markerColour = PedRace::colour(race);
        delete *ppCameraMarker;
        *ppCameraMarker = new PedCameraMarker(pParent, cameraLocalTransform, cameraType, markerColour);
        (*ppCameraMarker)->draw();
    }
    else
    {
        // Remove old warnings
        warnings_.erase(warnings_.begin(), warnings_.end());

        string msg("Warning : cannot set marker in current camera mode ");
        warnings_.push_back(msg);
    }
}

/* End EDITCAMR.CPP *************************************************/
