/*
 * C A M E R A S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/Cameras.hpp"
#include "device/KeyToCommandTranslator.hpp"
#include "machgui/IInputRegistry.hpp"
#include "mathex/Polygon2d.hpp"
#include "phys/Plans/MotionChunk.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "world4d/Entity/MotionControlledEntity.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Entity/Root.hpp"
#include "world4d/Sound/SoundManager.hpp"
#include "phys/MotionControl/FlyControl.hpp"
#include "phys/MotionControl/GroundFlyControl.hpp"
#include "phys/MotionControl/ZenithFlyControl.hpp"
#include "phys/Plans/MotionChunk.hpp"
#include "machlog/World/Camera.hpp"
#include "machlog/Races.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machlog/Actors/MotionSequencer.hpp"
#include "machlog/World/CameraMotionConstraints.hpp"
#include "machlog/Races.hpp"
#include "machlog/World/Planet.hpp"
#include "machphys/Machines/Machine.hpp"
#include "machphys/Machines/MachineData.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "envirnmt/PlanetEnvironment.hpp"
#include "render/Device.hpp"
#include "utility/LineTokeniser.hpp"
#include "gui/Event.hpp"
#include "system/VFS.hpp"

#include <memory>
#include <ranges>

static void readZenithDataFile(
    MATHEX_SCALAR* pZenithMinHeight,
    MATHEX_SCALAR* pZenithMaxHeight,
    MATHEX_SCALAR* pZenithElevation,
    MATHEX_SCALAR* pZenithEarMinHeight,
    MATHEX_SCALAR* pZenithEarMaxHeight);

MachCameras::MachCameras(W4dSceneManager* pSceneManager, W4dRoot* pRoot)
    : pSceneManager_(pSceneManager)
    , pRoot_(pRoot)
    , pCurrentCamera_(nullptr)
    , zenithMinimumHeight_(0.0)
    , zenithMaximumHeight_(1.0)
    , zenithMinimumEarHeight_(0.0)
    , zenithMaximumEarHeight_(1.0)
    , cameraMoved_(false)
    , groundCameraMoved_(true)
{
    pKeyTranslator_ = std::make_unique<DevKeyToCommandTranslator>();
    const auto addTranslation = [this](Command command, MachGui::BindId bindName)
    {
        const auto& trigger = MachGui::inputRegistry()->getBinds(bindName);
        pKeyTranslator_->addTranslation(DevKeyToCommand(command, &trigger));
    };
    addTranslation(Command::ZENITHVIEW, "view-use-zenith-camera"_bind);
    addTranslation(Command::GROUNDVIEW, "view-use-ground-camera"_bind);
    // The game uses "view-toggle-fpv" instead
    // addTranslation(Command::FIRSTPERSONVIEW, "view-use-in-head-camera"_bind);

    addTranslation(Command::SAVEVIEW1, "view-save-view-1"_bind);
    addTranslation(Command::SAVEVIEW2, "view-save-view-2"_bind);
    addTranslation(Command::SAVEVIEW3, "view-save-view-3"_bind);
    addTranslation(Command::SAVEVIEW4, "view-save-view-4"_bind);
    addTranslation(Command::RESTOREVIEW1, "view-restore-view-1"_bind);
    addTranslation(Command::RESTOREVIEW2, "view-restore-view-2"_bind);
    addTranslation(Command::RESTOREVIEW3, "view-restore-view-3"_bind);
    addTranslation(Command::RESTOREVIEW4, "view-restore-view-4"_bind);
    addTranslation(Command::RESTOREDEFAULTCAMERAPOS, "view-restore-default-pos"_bind);
    addTranslation(Command::RESTORELASTCAMERAPOS, "view-restore-last-pos"_bind);

    pKeyTranslator_->initEventQueue();

    TEST_INVARIANT;
}

MachCameras::~MachCameras()
{
    TEST_INVARIANT;

    DEBUG_STREAM(DIAG_NEIL, "MachCameras::~MachCameras()" << std::endl);
}

void MachCameras::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
    INVARIANT(pSceneManager_->currentCamera() == pCurrentCamera_);
}

void MachCameras::loadGame()
{
    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    MexTransform3d eyeTransform(MexEulerAngles(MexDegrees(45), 0.0, 0.0), MexPoint3d(0.0, 0.0, 2.0));
    MexTransform3d groundTransform(MexEulerAngles(MexDegrees(111), 0.0, 0.0), MexPoint3d(150.0, 150.0, 1.5));

    MATHEX_SCALAR zenithMinHeight = 20.0;
    MATHEX_SCALAR zenithMaxHeight = 250.0;
    MATHEX_SCALAR zenithElevation = 60.0;
    MATHEX_SCALAR zenithMinEarHeight = 20.0;
    MATHEX_SCALAR zenithMaxEarHeight = 250.0;

    readZenithDataFile(&zenithMinHeight, &zenithMaxHeight, &zenithElevation, &zenithMinEarHeight, &zenithMaxEarHeight);

    MexTransform3d zenithTransform(
        MexEulerAngles(MexDegrees(-120), MexDegrees(zenithElevation), 0.0),
        MexPoint3d(150.0, 150.0, 100.0));

    zenithMinimumHeight_ = zenithMinHeight;
    zenithMaximumHeight_ = zenithMaxHeight;
    zenithMinimumEarHeight_ = zenithMinEarHeight;
    zenithMaximumEarHeight_ = zenithMaxEarHeight;

    switch (races.cameraInfo(playerRace).type_)
    {
        case MachLogCamera::GROUND:
            groundTransform = races.cameraInfo(playerRace).position_;
            break;
        case MachLogCamera::ZENITH:
            zenithTransform = races.cameraInfo(playerRace).position_;
            break;
            DEFAULT_ASSERT_BAD_CASE((int)races.cameraInfo(playerRace).type_);
    }

    pFirstPersonCamera_ = std::make_unique<MachLogCamera>(pSceneManager_, pRoot_, eyeTransform, MachLogCamera::FIRST_PERSON);
    pGroundCamera_ = std::make_unique<MachLogCamera>(pSceneManager_, pRoot_, groundTransform, MachLogCamera::GROUND);
    pZenithCamera_ = std::make_unique<MachLogCamera>(pSceneManager_, pRoot_, zenithTransform, MachLogCamera::ZENITH);
    pFreeCamera_ = std::make_unique<MachLogCamera>(pSceneManager_, pRoot_, groundTransform, MachLogCamera::FREE_MOVING);

    pGroundConstraint_ = std::make_unique<MachLogGroundCameraMotionConstraint>(pGroundCamera_.get());
    pZenithConstraint_ = std::make_unique<MachLogZenithCameraMotionConstraint>(pZenithCamera_.get());
    pPlanetConstraint_ = std::make_unique<MachLogPlanetCameraConstraint>();

    pGroundControl_ = std::make_unique<PhysGroundFlyControl>(std::make_unique<W4dMotionControlledEntity>(pGroundCamera_.get()));
    pGroundControl_->setConstraint(pGroundConstraint_.get());
    pFirstPersonControl_ = std::make_unique<PhysFlyControl>(std::make_unique<W4dMotionControlledEntity>(pFirstPersonCamera_.get()));
    pFreeControl_ = std::make_unique<PhysFlyControl>(std::make_unique<W4dMotionControlledEntity>(pFreeCamera_.get()));
    pFreeControl_->setConstraint(pPlanetConstraint_.get());

    pZenithConstraint_->minHeight(zenithMinHeight);
    pZenithConstraint_->maxHeight(zenithMaxHeight);

    pZenithControl_ = std::make_unique<PhysZenithFlyControl>(std::make_unique<W4dMotionControlledEntity>(pZenithCamera_.get()));
    pZenithControl_->setConstraint(pZenithConstraint_.get());

    // disable the key translator for now because currently keys do not control machine.
    pFirstPersonControl_->disableInput();

    if (races.cameraInfo(playerRace).type_ == MachLogCamera::ZENITH)
    {
        // Initially use the zenith camera.
        pZenithControl_->enableInput();
        pCurrentCamera_ = pZenithCamera_.get();
        pSceneManager_->useCamera(pZenithCamera_.get());
        reduceFog();
    }
    else
    {
        // Initially use the ground camera.
        pGroundControl_->enableInput();
        pCurrentCamera_ = pGroundCamera_.get();
        pSceneManager_->useCamera(pGroundCamera_.get());
        restoreFog();
    }

    pFreeControl_->metresPerSecond(1);
    pFreeControl_->degreesPerSecond(10);

    // Ensure the cameras are in the correct domain
    pGroundCamera_->update();
    pFreeCamera_->update();
    pZenithCamera_->update();

    pGroundControl_->initEventQueue();
    pFreeControl_->initEventQueue();
    pZenithControl_->initEventQueue();
    pFirstPersonControl_->initEventQueue();

    // Fill save camera structures with info about initial camera position
    saveCamera(&save1_);
    saveCamera(&save2_);
    saveCamera(&save3_);
    saveCamera(&save4_);
    saveCamera(&startCameraPos_);
    saveCamera(&lastCameraPos_);

    cameraMoved_ = false;
    groundCameraMoved_ = true;
    resetFollowTarget();
}

void MachCameras::saveGame(PerOstream& outStream)
{
    if (isGroundCameraActive())
    {
        // Save camera type
        MachLogCamera::Type cameraType = MachLogCamera::GROUND;
        PER_WRITE_RAW_OBJECT(outStream, cameraType);
        MexTransform3d trans = pGroundCamera_->globalTransform();
        // Save camera pos
        PER_WRITE_RAW_OBJECT(outStream, trans);
    }
    else
    {
        // Save camera type
        MachLogCamera::Type cameraType = MachLogCamera::ZENITH;
        PER_WRITE_RAW_OBJECT(outStream, cameraType);

        // Save camera position data
        MATHEX_SCALAR zoomDistance;
        MATHEX_SCALAR x;
        MATHEX_SCALAR y;
        MexRadians heading;
        pZenithConstraint_->cameraPositionData(&zoomDistance, &x, &y, &heading);
        PER_WRITE_RAW_OBJECT(outStream, zoomDistance);
        PER_WRITE_RAW_OBJECT(outStream, x);
        PER_WRITE_RAW_OBJECT(outStream, y);
        PER_WRITE_RAW_OBJECT(outStream, heading);
    }

    // Write out saved camera positions
    PER_WRITE_RAW_OBJECT(outStream, save1_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, save1_.position_);
    PER_WRITE_RAW_OBJECT(outStream, save2_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, save2_.position_);
    PER_WRITE_RAW_OBJECT(outStream, save3_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, save3_.position_);
    PER_WRITE_RAW_OBJECT(outStream, save4_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, save4_.position_);
    PER_WRITE_RAW_OBJECT(outStream, startCameraPos_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, startCameraPos_.position_);
    PER_WRITE_RAW_OBJECT(outStream, lastCameraPos_.saved_);
    PER_WRITE_RAW_OBJECT(outStream, lastCameraPos_.position_);
}

void MachCameras::loadSavedGame(PerIstream& inStream)
{
    loadGame();

    // Load camera type
    int cameraType;
    PER_READ_RAW_OBJECT(inStream, cameraType);

    if (cameraType == MachLogCamera::GROUND)
    {
        // Switch to ground camera
        useGroundCamera();
        MexTransform3d trans;
        // Read camera pos
        PER_READ_RAW_OBJECT(inStream, trans);
        pGroundCamera_->globalTransform(trans);
    }
    else
    {
        // Switch to zenith camera
        useZenithCamera();

        // Read camera position data
        MATHEX_SCALAR zoomDistance;
        MATHEX_SCALAR x;
        MATHEX_SCALAR y;
        MexRadians heading;
        PER_READ_RAW_OBJECT(inStream, zoomDistance);
        PER_READ_RAW_OBJECT(inStream, x);
        PER_READ_RAW_OBJECT(inStream, y);
        PER_READ_RAW_OBJECT(inStream, heading);

        pZenithConstraint_->cameraPositionData(zoomDistance, x, y, heading);
    }

    // Read in saved camera positions
    PER_READ_RAW_OBJECT(inStream, (int&)save1_.saved_);
    PER_READ_RAW_OBJECT(inStream, save1_.position_);
    PER_READ_RAW_OBJECT(inStream, (int&)save2_.saved_);
    PER_READ_RAW_OBJECT(inStream, save2_.position_);
    PER_READ_RAW_OBJECT(inStream, (int&)save3_.saved_);
    PER_READ_RAW_OBJECT(inStream, save3_.position_);
    PER_READ_RAW_OBJECT(inStream, (int&)save4_.saved_);
    PER_READ_RAW_OBJECT(inStream, save4_.position_);
    PER_READ_RAW_OBJECT(inStream, (int&)startCameraPos_.saved_);
    PER_READ_RAW_OBJECT(inStream, startCameraPos_.position_);
    PER_READ_RAW_OBJECT(inStream, (int&)lastCameraPos_.saved_);
    PER_READ_RAW_OBJECT(inStream, lastCameraPos_.position_);
}

void MachCameras::unloadGame()
{
    pGroundControl_.reset();
    pFreeControl_.reset();
    pZenithControl_.reset();
    pFirstPersonControl_.reset();
    pGroundCamera_.reset();
    pFreeCamera_.reset();
    pZenithCamera_.reset();
    pFirstPersonCamera_.reset();

    pCurrentCamera_ = nullptr;
}

void MachCameras::useCamera(MachLogCamera* c)
{
    MachLogCamera* oldCamera = pCurrentCamera_;
    pCurrentCamera_ = c;
    pSceneManager_->useCamera(pCurrentCamera_);
    pCurrentCamera_->enable(oldCamera);

    TEST_INVARIANT;
}

MachLogCamera* MachCameras::currentCamera()
{
    PRE(pCurrentCamera_ != nullptr);

    return pCurrentCamera_;
}

bool MachCameras::processButtonEvent(const GuiKeyEvent& event)
{
    const DevButtonEvent& buttonEvent = event.buttonEvent();
    typedef DevKeyToCommand::CommandId CommandId;

    // Give motion control chance to accept buttonEvent
    bool processed = PhysMotionControl::processButtonEvent(buttonEvent);

    // Inidicate that camera has moved so that it can be saved in the update method.
    if (processed && (isGroundCameraActive() || isZenithCameraActive()))
    {
        cameraMoved_ = true;
        groundCameraMoved_ = true;
    }

    if (! processed && buttonEvent.action() == DevButtonEvent::PRESS)
    {
        // Now process all commands that motion control is not interested in
        CommandId commandId;
        processed = pKeyTranslator_->translate(buttonEvent, &commandId);

        if (processed)
        {
            switch (commandId)
            {
                case ZENITHVIEW:
                    useZenithCamera();
                    break;
                case GROUNDVIEW:
                    useGroundCamera();
                    break;
                case FIRSTPERSONVIEW:
                    use1stPersonCamera();
                    break;
                case SAVEVIEW1:
                    saveCamera(&save1_);
                    break;
                case SAVEVIEW2:
                    saveCamera(&save2_);
                    break;
                case SAVEVIEW3:
                    saveCamera(&save3_);
                    break;
                case SAVEVIEW4:
                    saveCamera(&save4_);
                    break;
                case RESTOREVIEW1:
                    restoreCamera(save1_);
                    break;
                case RESTOREVIEW2:
                    restoreCamera(save2_);
                    break;
                case RESTOREVIEW3:
                    restoreCamera(save3_);
                    break;
                case RESTOREVIEW4:
                    restoreCamera(save4_);
                    break;
                case RESTOREDEFAULTCAMERAPOS:
                    restoreCamera(startCameraPos_);
                    break;
                case RESTORELASTCAMERAPOS:
                    restoreCamera(lastCameraPos_);
                    break;
            }
        }
    }

    return processed;
}

void MachCameras::switchToZenith(const MexPoint3d& lookAt)
{
    // Dense fog in the zenith view is silly because it obscures the
    // ground.  Cruft the value to make it less dense.
    reduceFog();

    pZenithControl_->snapTo(MexTransform3d(lookAt));
    useCamera(pZenithCamera_.get());
    pZenithCamera_->update();
    pZenithControl_->enableInput();
}

void MachCameras::updateCameras()
{
    PhysMotionControlWithTrans *pControl{};
    if (isZenithCameraActive())
        pControl = pZenithControl_.get();
    else if (isGroundCameraActive())
        pControl = pGroundControl_.get();

    if (pControl)
    {
        using ControlCommand = PhysMotionControlWithTrans::Command;
        static const ControlCommand moveCommands[] = {
            ControlCommand::FOWARD,
            ControlCommand::BACKWARD,
            ControlCommand::SLIDE_LEFT,
            ControlCommand::SLIDE_RIGHT,
        };

        if (std::ranges::any_of(moveCommands, [pControl](ControlCommand commandId) -> bool {
            return pControl->isCommandOn(commandId);
        }))
        {
            resetFollowTarget();
        }
    }

    if (pFollowTarget_)
    {
        if (pFollowTarget_->selectionState() != MachLog::SELECTED)
        {
            resetFollowTarget();
        }
        else
        {
            double heading{};
            double climb{};
            double frameTimer{};

            if (isZenithCameraActive())
            {
                frameTimer = pZenithControl_->frameTimerRef().time();
                heading = pZenithControl_->motionRef().heading();
                climb = pZenithControl_->motionRef().climb();

                MexTransform3d actorTrans = pFollowTarget_->globalTransform();
                MexPoint3d actorPos = actorTrans.position();

                pZenithControl_->snapTo(actorPos);

                pZenithControl_->frameTimerRef().time(frameTimer);
                pZenithControl_->motionRef().heading(heading);
                pZenithControl_->motionRef().climb(climb);
            }
            else
            {
                internalLookAt(*pFollowTarget_);
            }
        }
    }

    if (isGroundCameraActive())
    {
        pGroundControl_->update();
        pGroundCamera_->update();
    }
    else if (is1stPersonCameraActive())
    {
        pFirstPersonCamera_->update();
    }
    else if (isZenithCameraActive())
    {
        pZenithControl_->update();
        pZenithCamera_->update();
    }

    if (cameraMoved_)
    {
        cameraMoved_ = false;
        saveCamera(&lastCameraPos_);
    }
}

void MachCameras::reduceFog()
{
    MachLogPlanet& malPlanet = MachLogPlanet::instance();

    if (malPlanet.hasSurface())
    {
        MachPhysPlanetSurface* physPlanet = malPlanet.surface();

        if (physPlanet->hasEnvironment())
        {
            EnvPlanetEnvironment& env = physPlanet->environment();
            env.fogZenith(true);
        }
    }
}

void MachCameras::restoreFog()
{
    MachLogPlanet& malPlanet = MachLogPlanet::instance();

    if (malPlanet.hasSurface())
    {
        MachPhysPlanetSurface* physPlanet = malPlanet.surface();

        if (physPlanet->hasEnvironment())
        {
            EnvPlanetEnvironment& env = physPlanet->environment();
            env.fogZenith(false);
        }
    }
}

void MachCameras::saveCamera(CameraSave* pCameraSave)
{
    if (pGroundControl_->inputEnabled())
    {
        pCameraSave->saved_ = CameraSave::GROUNDVIEW;
        pCameraSave->position_ = pGroundCamera_->globalTransform();
        pCameraSave->heightDelta_ = pGroundConstraint_->zTerrainDelta();
    }
    else if (pZenithControl_->inputEnabled())
    {
        pCameraSave->saved_ = CameraSave::ZENITHVIEW;

        // We need tp store the actual zenith view parameters.
        // We do this using the transform.
        MATHEX_SCALAR zoomDistance;
        MATHEX_SCALAR x;
        MATHEX_SCALAR y;
        MexRadians heading;
        pZenithConstraint_->cameraPositionData(&zoomDistance, &x, &y, &heading);
        MexTransform3d zoomStoreTransform(MexEulerAngles(heading, 0.0, 0.0), MexPoint3d(x, y, zoomDistance));
        pCameraSave->position_ = zoomStoreTransform;
    }
}

void MachCameras::restoreCamera(const CameraSave& cameraSave)
{
    resetFollowTarget();

    if (cameraSave.saved_ == CameraSave::GROUNDVIEW)
    {
        restoreFog();

        pGroundConstraint_->zTerrainDelta(cameraSave.heightDelta_);
        pGroundControl_->snapTo(cameraSave.position_);
        useCamera(pGroundCamera_.get());
        pGroundCamera_->update();
        pGroundControl_->enableInput();
    }
    else if (cameraSave.saved_ == CameraSave::ZENITHVIEW)
    {
        reduceFog();

        // Extract the zenith view parameters from the saved transform
        MATHEX_SCALAR zoomDistance = cameraSave.position_.position().z();
        MATHEX_SCALAR x = cameraSave.position_.position().x();
        MATHEX_SCALAR y = cameraSave.position_.position().y();
        MexEulerAngles angles;
        cameraSave.position_.rotation(&angles);
        MexRadians heading = angles.azimuth();

        // Use the zenith motion constraint to update the view position parameters
        pZenithConstraint_->cameraPositionData(zoomDistance, x, y, heading);

        // Make sure we use and update this camera
        useCamera(pZenithCamera_.get());
        pZenithCamera_->update();
        pZenithControl_->enableInput();
    }
}

// Works out position of zenith camera so that the centre of the screen is the
// co-ords specified by lookAt.
MexPoint3d MachCameras::zenithLookAt(const MexPoint3d& lookAt)
{
    MexTransform3d zenithPos = pZenithCamera_->globalTransform();
    MATHEX_SCALAR zenithZ = zenithPos.position().z();

    MexVec3 xBasis = zenithPos.xBasis();
    MexVec3 zVec(0, 0, 1);
    MATHEX_SCALAR dotRes = xBasis.dotProduct(zVec);

    MATHEX_SCALAR scale = -zenithZ / dotRes;

    xBasis *= scale;

    MexPoint3d newZenithPos = lookAt;
    newZenithPos -= xBasis;

    return newZenithPos;
}

// Make camera move to new position ( assuming camera can move without
// ending up inside an obstacle etc ).
void MachCameras::moveTo(const MexPoint2d& newPos)
{
    resetFollowTarget();

    pZenithControl_->snapTo(MexPoint3d(newPos.x(), newPos.y(), pZenithCamera_->globalTransform().position().z()));
    pGroundControl_->snapTo(MexPoint3d(newPos.x(), newPos.y(), pGroundCamera_->globalTransform().position().z()));
    pFreeControl_->snapTo(MexPoint3d(newPos.x(), newPos.y(), pFreeCamera_->globalTransform().position().z()));

    // Make sure camera domain is updated.
    updateCameras();
}

void MachCameras::setFollowTarget(MachActor* pActor)
{
    if (pFollowTarget_ == pActor)
        return;

    if (pFollowTarget_)
    {
        pFollowTarget_->detach(this);
    }

    pFollowTarget_ = pActor;

    if (pActor)
    {
        pFollowTarget_->attach(this);
    }
}

void MachCameras::resetFollowTarget()
{
    setFollowTarget(nullptr);
}

// Make camera move into a position where it can look at the MachActor.
void MachCameras::lookAt(const MachActor& actor)
{
    resetFollowTarget();
    internalLookAt(actor);
}

// Make camera move into a position where it can look at the MachActor.
void MachCameras::internalLookAt(const MachActor& actor)
{
    MexTransform3d actorTrans = actor.globalTransform();
    MexPoint3d actorPos = actorTrans.position();

    if (isZenithCameraActive())
    {
        switchToZenith(actorPos);
    }
    else if (isGroundCameraActive())
    {
        if (actor.objectIsMachine()) // Only stay in ground view if actor is a machine
        {
            // Don't want to position camera directly on top of the actor, so
            // work out a position just behind the actor
            MexTransform3d newPos;
            bool machineIsGlider = false;

            // To get gliders into view in ground camera we need to move further behind the machine
            if (actor.asMachine().physMachine().machineData().locomotionType() == MachPhys::GLIDER)
            {
                actor.globalTransform().transform(MexPoint3d(-40.0, 0.0, 0.0), &newPos);
                machineIsGlider = true;
            }
            else
            {
                actor.globalTransform().transform(MexPoint3d(-25.0, 0.0, 0.0), &newPos);
            }

            // Check new position...
            if (pGroundCamera_->newPositionIsValid(newPos))
            {
                //  Make sure that the roll and elevation for the ground camera are zero
                MexEulerAngles angles;
                newPos.rotation(&angles);
                angles.roll(0.0);
                angles.elevation(0.0);
                newPos.rotation(angles);

                if (machineIsGlider)
                {
                    pGroundConstraint_->zTerrainDelta(pGroundConstraint_->maxHeight());
                }
                else
                {
                    pGroundConstraint_->zTerrainDelta(pGroundConstraint_->minHeight() + 1.0);
                }

                pGroundControl_->snapTo(newPos);
                pGroundCamera_->update(); // Make sure camera is in correct domain
            }
            else // Ground camera couldn't be positioned behind the actor, switch to zenith instead
            {
                switchToZenith(actorPos);
            }
        }
        else // switch to zenith camera to view construction
        {
            switchToZenith(actorPos);
        }
    }
}

void MachCameras::scroll(ScrollDir scrollDir, const GuiMouseEvent& event)
{
    if (pCurrentCamera_ == pZenithCamera_.get())
    {
        switch (scrollDir)
        {
            case LEFT:
                if (event.rightButton() == Gui::PRESSED)
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::ROTATE_LEFT);
                else
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::SLIDE_LEFT);
                break;
            case RIGHT:
                if (event.rightButton() == Gui::PRESSED)
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::ROTATE_RIGHT);
                else
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::SLIDE_RIGHT);
                break;
            case UP:
                if (event.rightButton() == Gui::PRESSED)
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::DOWN);
                else
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::FOWARD);
                break;
            case DOWN:
                if (event.rightButton() == Gui::PRESSED)
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::UP);
                else
                    pZenithControl_->forceCommandOn(PhysMotionControlWithTrans::BACKWARD);
                break;
        }
    }
    else if (pCurrentCamera_ == pGroundCamera_.get())
    {
        switch (scrollDir)
        {
            case LEFT:
                if (event.rightButton() == Gui::PRESSED)
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::SLIDE_LEFT);
                else
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::ROTATE_LEFT);
                break;
            case RIGHT:
                if (event.rightButton() == Gui::PRESSED)
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::SLIDE_RIGHT);
                else
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::ROTATE_RIGHT);
                break;
            case UP:
                if (event.rightButton() == Gui::PRESSED)
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::UP);
                else
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::FOWARD);
                break;
            case DOWN:
                if (event.rightButton() == Gui::PRESSED)
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::DOWN);
                else
                    pGroundControl_->forceCommandOn(PhysMotionControlWithTrans::BACKWARD);
                break;
        }
    }
}

void MachCameras::scrollWithWheel(const Gui::ScrollState wheelDir, const double step)
{
    constexpr auto zoomIn = Gui::ScrollState::SCROLL_UP;
    constexpr auto zoomOut = Gui::ScrollState::SCROLL_DOWN;

    if (pCurrentCamera_ == pZenithCamera_.get())
    {
        MATHEX_SCALAR zoomDistance{};
        MATHEX_SCALAR x{};
        MATHEX_SCALAR y{};
        MexRadians heading{};
        pZenithConstraint_->cameraPositionData(&zoomDistance, &x, &y, &heading);

        constexpr double zoomFraction = 1.0 / 8;
        if (wheelDir == zoomIn)
            zoomDistance *= (1.0 - zoomFraction);
        else if (wheelDir == zoomOut)
            zoomDistance *= (1.0 + zoomFraction);

        zoomDistance = std::clamp(zoomDistance, pZenithConstraint_->minZoomDistance(),
                                               pZenithConstraint_->maxZoomDistance());
        pZenithConstraint_->cameraPositionData(zoomDistance, x, y, heading);
    }
    else if (pCurrentCamera_ == pGroundCamera_.get())
    {
        // Adjust height above terrain proportionally
        MATHEX_SCALAR delta = pGroundConstraint_->zTerrainDelta();

        constexpr double zoomFraction = 0.1;
        if (wheelDir == zoomIn)
            delta *= (1.0 + zoomFraction);
        else if (wheelDir == zoomOut)
            delta *= (1.0 - zoomFraction);

        delta = std::clamp(delta, static_cast<MATHEX_SCALAR>(pGroundConstraint_->minHeight()),
                                  static_cast<MATHEX_SCALAR>(pGroundConstraint_->maxHeight()));
        pGroundConstraint_->zTerrainDelta(delta);
    }
}

bool MachCameras::isZenithCameraActive() const
{
    return (pCurrentCamera_ == pZenithCamera_.get());
}

bool MachCameras::isGroundCameraActive() const
{
    return (pCurrentCamera_ == pGroundCamera_.get());
}

bool MachCameras::is1stPersonCameraActive() const
{
    return (pCurrentCamera_ == pFirstPersonCamera_.get());
}

void MachCameras::freezeMotion()
{
    if (! pZenithControl_->motionFrozen())
        pZenithControl_->freezeMotion();
    if (! pGroundControl_->motionFrozen())
        pGroundControl_->freezeMotion();
    if (! pFreeControl_->motionFrozen())
        pFreeControl_->freezeMotion();
}

void MachCameras::resumeMotion()
{
    if (pZenithControl_->motionFrozen())
        pZenithControl_->resumeMotion();
    if (pGroundControl_->motionFrozen())
        pGroundControl_->resumeMotion();
    if (pFreeControl_->motionFrozen())
        pFreeControl_->resumeMotion();
}

bool MachCameras::motionFrozen() const
{
    bool frozen = false;

    if (isZenithCameraActive())
        frozen = pZenithControl_->motionFrozen();
    else if (isGroundCameraActive())
        frozen = pGroundControl_->motionFrozen();
    else if (is1stPersonCameraActive())
        frozen = pFirstPersonControl_->motionFrozen();

    return frozen;
}

std::ostream& operator<<(std::ostream& o, const MachCameras& t)
{

    o << "MachCameras " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachCameras " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

//  TBD: Make this into a proper member function (don't want to touch
//  the header file at the moment). Alternatively we might drop the
//  settable heights and viewing angle anyway.

void readZenithDataFile(
    MATHEX_SCALAR* pZenithMinHeight,
    MATHEX_SCALAR* pZenithMaxHeight,
    MATHEX_SCALAR* pZenithElevation,
    MATHEX_SCALAR* pZenithEarMinHeight,
    MATHEX_SCALAR* pZenithEarMaxHeight)
{
    std::string configFile = System::findFile("data/zenith.dat");
    const SysPathName fileName(configFile);

    ASSERT_FILE_EXISTS(fileName.c_str());
    std::unique_ptr<std::istream> pIstream
        = std::unique_ptr<std::istream>(new std::ifstream(fileName.c_str(), std::ios::in | std::ios::in));

    UtlLineTokeniser tokeniser(*pIstream, fileName);

    while (! tokeniser.finished())
    {
        ASSERT_INFO(tokeniser.tokens().size());
        ASSERT(tokeniser.tokens().size() == 2, "");

        MATHEX_SCALAR value = atof(tokeniser.tokens()[1].c_str());

        if (tokeniser.tokens()[0] == "MIN_HEIGHT")
        {
            *pZenithMinHeight = value;
        }
        else if (tokeniser.tokens()[0] == "MAX_HEIGHT")
        {
            *pZenithMaxHeight = value;
        }
        else if (tokeniser.tokens()[0] == "ELEVATION")
        {
            *pZenithElevation = value;
        }
        else if (tokeniser.tokens()[0] == "MIN_EAR_HEIGHT")
        {
            *pZenithEarMinHeight = value;
        }
        else if (tokeniser.tokens()[0] == "MAX_EAR_HEIGHT")
        {
            *pZenithEarMaxHeight = value;
        }
        else if (tokeniser.tokens()[0] == "SCALE_FACTOR_MAX")
        {
            W4dSoundManager::instance().setMaxScaleFactor(value);
        }
        else if (tokeniser.tokens()[0] == "SCALE_FACTOR_MIN")
        {
            W4dSoundManager::instance().setMinScaleFactor(value);
        }
        else
        {
            ASSERT_INFO(tokeniser.tokens()[0]);
            ASSERT_FAIL("Illegal token");
        }

        tokeniser.parseNextLine();
    }
}

void MachCameras::lookAt(const MexPoint2d& newPos)
{
    resetFollowTarget();

    if (isZenithCameraActive())
    {
        switchToZenith(newPos);
    }
    else if (isGroundCameraActive())
    {
        pGroundControl_->snapTo(MexPoint3d(newPos.x(), newPos.y(), pGroundCamera_->globalTransform().position().z()));
        pGroundCamera_->update(); // Make sure camera is in correct domain
    }
    else if (is1stPersonCameraActive())
    {
        pFirstPersonControl_->snapTo(
            MexPoint3d(newPos.x(), newPos.y(), pFirstPersonCamera_->globalTransform().position().z()));
        pFirstPersonCamera_->update(); // Make sure camera is in correct domain
    }
}

MATHEX_SCALAR MachCameras::zenithMinimumHeight() const
{
    return zenithMinimumHeight_;
}

MATHEX_SCALAR MachCameras::zenithMaximumHeight() const
{
    return zenithMaximumHeight_;
}

MATHEX_SCALAR MachCameras::zenithMinimumEarHeight() const
{
    return zenithMinimumEarHeight_;
}

MATHEX_SCALAR MachCameras::zenithMaximumEarHeight() const
{
    return zenithMaximumEarHeight_;
}

void MachCameras::use1stPersonCamera()
{
    resetFollowTarget();

    useCamera(pFirstPersonCamera_.get());
    pFirstPersonCamera_->update();
    pFirstPersonControl_->enableInput();
    restoreFog();
    groundCameraMoved_ = true;
}

void MachCameras::useGroundCamera()
{
    if (! isGroundCameraActive())
    {
        // Switching from zenith camera, store it's position. If we switch straight
        // back to zenith then the camera will jump back to it's last position otherwise
        // we will have to calculate a new zenith camera position based on the ground
        // camera position which is less accurate.
        if (isZenithCameraActive())
        {
            saveCamera(&lastZenithPos_);
            groundCameraMoved_ = false;
        }

        // Get current camera position
        MexTransform3d cameraTrans = pCurrentCamera_->globalTransform();

        // Remove any roll and pitch ( caused by machine going up hill or exploding )
        MexEulerAngles angles = cameraTrans.rotationAsEulerAngles();
        angles.roll(0);
        angles.elevation(0);

        cameraTrans.rotation(angles);

        // Set ground camera to new position.
        pGroundControl_->snapTo(cameraTrans);
        useCamera(pGroundCamera_.get());
        pGroundCamera_->update();
        pGroundControl_->enableInput();
        restoreFog();
    }
}

void MachCameras::useZenithCamera()
{
    if (!isZenithCameraActive())
    {
        if (isGroundCameraActive())
        {
            if (pFollowTarget_)
            {
                MexTransform3d actorTrans = pFollowTarget_->globalTransform();
                MexPoint3d actorPos = actorTrans.position();
                switchToZenith(actorPos);
            }
            else if (!groundCameraMoved_)
            {
                // If the ground camera hasn't moved then put zenith camera back to it's
                // last position, else work out a new zenith position based on the new ground
                // position.
                restoreCamera(lastZenithPos_);
            }
            else
            {
                MexTransform3d newPos;
                // Get zenith camera position data
                MATHEX_SCALAR zoomDistance;
                MATHEX_SCALAR x;
                MATHEX_SCALAR y;
                MexRadians heading;
                pZenithConstraint_->cameraPositionData(&zoomDistance, &x, &y, &heading);

                pCurrentCamera_->globalTransform().transform(MexPoint3d(zoomDistance * 0.7, 0.0, 0.0), &newPos);
                switchToZenith(newPos.position());
            }
        }
        else
        {
            MexPoint3d newPos = pCurrentCamera_->globalTransform().position();
            switchToZenith(newPos);
        }
    }
}

Gui::Coord MachCameras::positionOnTerrainThatZenithCameraIsLookingAt() const
{
    PRE(isZenithCameraActive());

    MATHEX_SCALAR zoomDistance;
    MATHEX_SCALAR x;
    MATHEX_SCALAR y;
    MexRadians heading;
    pZenithConstraint_->cameraPositionData(&zoomDistance, &x, &y, &heading);

    Gui::Coord pos(x, y);

    return pos;
}

MachCameras::ZenithCameraData MachCameras::zenithCameraData() const
{
    PRE(isZenithCameraActive());

    ZenithCameraData data{};
    pZenithConstraint_->cameraPositionData(&data.zoomDistance, &data.x, &data.y, &data.heading);
    return data;
}

void MachCameras::zenithCameraData(const ZenithCameraData& data)
{
    PRE(isZenithCameraActive());

    pZenithConstraint_->cameraPositionData(data.zoomDistance, data.x, data.y, data.heading);
    pZenithCamera_->update();
}

void MachCameras::setGroundCameraPosition(MATHEX_SCALAR x, MATHEX_SCALAR y, MATHEX_SCALAR z)
{
    MachPhysPlanetSurface* pSurface = MachLogPlanet::instance().surface();
    const MATHEX_SCALAR terrainZ = pSurface->terrainHeight(x, y, pGroundCamera_->floors());
    const MATHEX_SCALAR delta = z - terrainZ;
    pGroundConstraint_->zTerrainDelta(delta);

    MexTransform3d xform = pGroundCamera_->globalTransform();
    xform.position(MexPoint3d(x, y, z));
    pGroundControl_->snapTo(xform);
    pGroundCamera_->update();
}

void MachCameras::reversePitchUpDownKeys(bool newValue)
{
    pGroundControl_->reversePitchUpDownKeys(newValue);
}

bool MachCameras::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData)
{
    bool stayAttached = true;

    if (pSubject != pFollowTarget_)
        return stayAttached;

    if (event != W4dSubject::DELETED)
        return stayAttached;

    resetFollowTarget();
    stayAttached = false;

    return stayAttached;
}

void MachCameras::domainDeleted(W4dDomain*)
{
    resetFollowTarget();
}

/* End CAMERAS.CPP **************************************************/
