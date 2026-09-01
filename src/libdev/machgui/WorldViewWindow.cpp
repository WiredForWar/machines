/*
 * W O R L D V I E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/WorldViewWindow.hpp"

#include "machgui/IInputRegistry.hpp"
#include "machgui/PausedImage.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/gui.hpp"
#include "machgui/commands/command.hpp"
#include "machgui/Cameras.hpp"
#include "machgui/ControlPanelAddOn.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "machlog/Races.hpp"
#include "machlog/World/PlanetDomains.hpp"
#include "machlog/Actors/Actor.hpp"
#include "gui/Event.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "world4d/Scene/Camera.hpp"
#include "world4d/Scene/Domain.hpp"
#include "world4d/Entity/Entity.hpp"
#include "world4d/Entity/Generic.hpp"
#include "world4d/Entity/BoundingBoxSelector.hpp"
#include "render/Device.hpp"
#include "mathex/Point2d.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/Line3d.hpp"
#include "mathex/Transform3d.hpp"
#include "mathex/Epsilon.hpp"
#include "utility/Id.hpp"
#include "system/ConfigVariables.hpp"
#include "system/PathName.hpp"
#include "gui/GuiPainter.hpp"
#include "device/KeyToCommandTranslator.hpp"
#include "render/Camera.hpp"
#include "sim/Manager.hpp"

#include <utility>

MachWorldViewWindow::MachWorldViewWindow(
    MachInGameScreen* pParent,
    const Gui::Boundary& relativeBox,
    MachCameras* pCameras)
    : GuiDisplayable(pParent, relativeBox)
    , pInGameScreen_(pParent)
    , haveHighlightedActor_(false)
    , pCameras_(pCameras)
    , mouseButtonPressed_(false)
    , rubberBandCamera_(INVALID)
    , rubberBanding_(false)
{
    pKeyTranslator_ = new DevKeyToCommandTranslator();
    const auto addTranslation
        = [](DevKeyToCommandTranslator* pKeyTranslator_, DevKeyToCommand::CommandId command, MachGui::BindId bindId) {
        const auto& trigger = MachGui::inputRegistry()->getBinds(bindId);
        pKeyTranslator_->addTranslation(DevKeyToCommand(command, &trigger));
    };

    addTranslation(pKeyTranslator_, SELECT_ONSCREEN_MACHINES, "select-visible-machines"_bind);
    addTranslation(pKeyTranslator_, SELECT_ONSCREEN_CONSTRUCTIONS, "select-visible-constructions"_bind);
    addTranslation(pKeyTranslator_, SELECT_ONSCREEN_MACHINES_KEEP_SEL, "add-visible-machines"_bind);
    addTranslation(pKeyTranslator_, SELECT_ONSCREEN_CONSTRUCTIONS_KEEP_SEL, "add-visible-constructions"_bind);

    selectedEntities_.reserve(20);

    new MachGuiPausedImage(this, pParent);

    useFastSecondDisplay(false);

    redrawEveryFrame(true);
}

MachWorldViewWindow::~MachWorldViewWindow()
{
    delete pKeyTranslator_;
}

// virtual
void MachWorldViewWindow::doHandleMouseClickEvent(const GuiMouseEvent& event)
{
    PRE(pCameras_);

    // Only interested in left mouse clicks over the window
    mouseButtonPressed_ = event.leftButton() == Gui::PRESSED;

    if (mouseButtonPressed_)
    {
        // get the 3d line
        Gui::Coord absCoord(event.coord());
        absCoord += absoluteBoundary().minCorner();
        startRubberBand_ = absCoord;

        // Store camera used at beginning of rubber banding operation
        if (pCameras_->isZenithCameraActive())
            rubberBandCamera_ = ZENITH;
        else if (pCameras_->isGroundCameraActive())
            rubberBandCamera_ = GROUND;
        else
            rubberBandCamera_ = INVALID;

        MexLine3d cursorLine = cameraThroughCursorLine(absCoord, 1000.0);

        // If we are rubber banding in zenith mode then we need to fix the
        // rubber band start point to a point at ground level in the planet.
        if (rubberBandCamera_ == ZENITH)
        {
            // Work out the 3D position when the cursor line hits z = 0
            MexVec3 cursorLineUnitVec = cursorLine.unitDirectionVector();
            MATHEX_SCALAR ratio = cursorLine.end1().z() / cursorLineUnitVec.z();
            MATHEX_SCALAR x2 = cursorLine.end1().x() - (ratio * cursorLineUnitVec.x());
            MATHEX_SCALAR y2 = cursorLine.end1().y() - (ratio * cursorLineUnitVec.y());

            startRubberBand3DPos_ = MexPoint3d(x2, y2, 0);
        }
    }
    else
    {
        if (rubberBanding_)
        {
            bool keepSelection = DevKeyboard::instance().ctrlPressed() || DevKeyboard::instance().shiftPressed();
            updateActorsSelectedViaRubberBand(false, keepSelection);
            pCameras_->resumeMotion();
            rubberBanding_ = false;
        }
        else
        {
            Gui::Coord absCoord(event.coord());
            absCoord += absoluteBoundary().minCorner();

            // No rubber banding was taking place so let the commands respond to
            // the mouse button release
            MexLine3d cursorLine = cameraThroughCursorLine(absCoord, 1000.0);

            // Dispatch the click
            dispatchCursor(cursorLine, true, event.isCtrlPressed(), event.isShiftPressed(), event.isAltPressed());
        }
    }
}

MexLine3d MachWorldViewWindow::cameraThroughCursorLine(const Gui::Coord& cursorScreenPoint, MATHEX_SCALAR length) const
{

    // Get the scene manager
    W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();

    // The viewport must be correctly set to correspond to this window before
    // we can use the RenDevice methods.
    pInGameScreen_->setWorldViewViewport();

    // Hence get corresponding world position
    MexPoint3d cursorWorldPos = sceneManager.pDevice()->screenToCamera(cursorScreenPoint);

    // Reset the viewport correctly for GUI drawing.  TBD: a save/restore or
    // push/pop idiom would be much more robust.
    pInGameScreen_->setGuiViewport();

    // Construct a line from the camera origin through the cursor point in world coordinates
    const MexTransform3d& cameraTransform = sceneManager.currentCamera()->globalTransform();
    MexPoint3d cameraOrigin(cameraTransform.position());
    cameraTransform.transform(&cursorWorldPos);
    MexLine3d shortLine(cameraOrigin, cursorWorldPos);

    // Now create one of length 1000m
    MexPoint3d farPoint = shortLine.pointAtDistance(length);
    MexLine3d theLine(cameraOrigin, farPoint, length);

    return theLine;
}

void MachWorldViewWindow::dispatchCursor(
    const MexLine3d& cursorLine,
    bool click,
    bool ctrlPressed,
    bool shiftPressed,
    bool altPressed)
{
    // Get the active command
    MachGuiCommand& activeCommand = pInGameScreen_->activeCommand();

    // Get the scene manager
    W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();

    // Get the domain in which the camera is located
    W4dDomain* pCameraDomain = sceneManager.currentCamera()->containingDomain();

    // Default 2d cursor is arrow
    MachGui::Cursor2dType cursor2d = MachGui::MENU_CURSOR;

    // Get the pointer of any currently highlighted actor
    MachActor* pOldHighlightedActor = nullptr;
    MachActor* pNewHighlightedActor = nullptr;
    if (haveHighlightedActor_)
    {
        MachLogRaces& races = MachLogRaces::instance();
        if (races.actorExists(highLightedActorId_))
            pOldHighlightedActor = &races.actor(highLightedActorId_);
    }

    // Find any entity intersecting the line
    W4dEntity* pEntity;
    MATHEX_SCALAR distance = 0.0;
    bool hit = false;

    hit = pCameraDomain->findNearerEntity(
        cursorLine,
        cursorLine.length(),
        W4dEntity::nextCheckId(),
        W4dEntity::Accuracy::Parts,
        &pEntity,
        &distance,
        pInGameScreen_->cursorFilter());

    if (distance <= 0.0
        || distance > (RenDevice::current()->fogEnd() * 0.9) /* After 90% fog distance things aren't visible*/)
    {
        hit = false;
    }

    auto onNothingHovered = [&]()
    {
        if (Config::devMode.get())
        {
            const MexPoint3d hitPoint = cursorLine.pointAtDistance(distance);
            pInGameScreen_->setCursorPromptTextToPos(MexPoint2d(hitPoint.x(), hitPoint.y()));
        }
        else
        {
            pInGameScreen_->clearCursorPromptText();
        }
    };

    // If we got something, dispatch it
    if (hit)
    {
        // Check for a hit on an actor ( only if we are not in FOW )
        UtlId hitId = pEntity->id();
        MachLogRaces& races = MachLogRaces::instance();

        if (! MachGuiCommand::cursorInFogOfWar() && races.actorExists(hitId))
        {
            // Get the picked actor
            MachActor& hitActor = races.actor(hitId);

            // Check that actor has not recently been blown up
            // so that bounding box isn't put round an explosion
            if (! hitActor.isDead())
            {
                // Display prompt text for actor
                pInGameScreen_->displayActorPromptText(&hitActor);

                // Dispatch the hit to the active command
                if (click)
                {
                    // Can't dispatch commands if game is paused or network is busy
                    if (SimManager::instance().isSuspended() || pInGameScreen_->isNetworkStuffed())
                    {
                        // Only allow selecting when paused
                        if (activeCommand.cursorOnActor(&hitActor, ctrlPressed, shiftPressed, altPressed)
                            == MachGui::SELECT_CURSOR)
                        {
                            activeCommand.pickOnActor(&hitActor, ctrlPressed, shiftPressed, altPressed);
                        }
                        else
                        {
                            MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
                        }
                    }
                    else
                    {
                        activeCommand.pickOnActor(&hitActor, ctrlPressed, shiftPressed, altPressed);
                    }
                }
                else
                {
                    cursor2d = activeCommand.cursorOnActor(&hitActor, ctrlPressed, shiftPressed, altPressed);
                }

                // If the actor under the cursor is not selected, highlight it
                if (hitActor.selectionState() != MachLog::SELECTED
                    && hitActor.selectableType() == MachLog::FULLY_SELECTABLE)
                    pNewHighlightedActor = &hitActor;
            }
        }
        else
        {
            onNothingHovered();

            // Assume hit on terrain. Get the point at the hit distance.
            MexPoint3d hitPoint = cursorLine.pointAtDistance(distance);

            // Add the 6 cm drop value on the terrain
            hitPoint.z(hitPoint.z() + 0.06);

            // Dispatch it
            if (click)
            {
                // Can't issue commands if game is paused or network is busy
                if (SimManager::instance().isSuspended() || pInGameScreen_->isNetworkStuffed())
                {
                    MachGuiSoundManager::instance().playSound("gui/sounds/clickbad.wav");
                }
                else
                {
                    activeCommand.pickOnTerrain(hitPoint, ctrlPressed, shiftPressed, altPressed);
                }
            }
            else
            {
                cursor2d = activeCommand.cursorOnTerrain(hitPoint, ctrlPressed, shiftPressed, altPressed);
            }
        }
    }
    else
    {
        onNothingHovered();
    }

    // Update the highlight state of any new/previous actor
    if (pOldHighlightedActor && pOldHighlightedActor != pNewHighlightedActor
        && pOldHighlightedActor->selectionState() == MachLog::HIGHLIGHTED)
    {
        pOldHighlightedActor->selectionState(MachLog::NOT_SELECTED);
        pInGameScreen_->removeHighlightedActor();
    }

    haveHighlightedActor_ = pNewHighlightedActor != nullptr;

    if (haveHighlightedActor_)
    {
        // Cache the highlighted id so we know which one to unhighlight next time
        highLightedActorId_ = pNewHighlightedActor->id();

        // Change to highlight state if not already in that state
        if (pNewHighlightedActor->selectionState() != MachLog::HIGHLIGHTED)
        {
            pNewHighlightedActor->selectionState(MachLog::HIGHLIGHTED);
        }

        pInGameScreen_->highlightActor(pNewHighlightedActor);
    }

    // If not a click, update the 2d cursor shape
    if (! click)
        pInGameScreen_->cursor2d(cursor2d);
}

// virtual
void MachWorldViewWindow::doHandleMouseEnterEvent(const GuiMouseEvent& event)
{
    // Action same as for position update
    doHandleContainsMouseEvent(event);
}

// virtual
void MachWorldViewWindow::doHandleMouseExitEvent(const GuiMouseEvent&)
{
    // Switch to menu cursor
    pInGameScreen_->cursor2d(MachGui::MENU_CURSOR);
    pInGameScreen_->clearCursorPromptText();

    // Remove highlight from around actor
    if (haveHighlightedActor_)
    {
        MachLogRaces& races = MachLogRaces::instance();
        if (races.actorExists(highLightedActorId_))
            races.actor(highLightedActorId_).selectionState(MachLog::NOT_SELECTED);
        pInGameScreen_->removeHighlightedActor();
        pInGameScreen_->clearCursorPromptText();
        haveHighlightedActor_ = false;
    }
}

// virtual
void MachWorldViewWindow::doHandleMouseScrollEvent(GuiMouseEvent* event)
{
    pCameras_->scrollWithWheel(event->scrollDirection(), 15.0);
    event->accept();
}

// virtual
void MachWorldViewWindow::doHandleContainsMouseEvent(const GuiMouseEvent& event)
{
    // get the 3d line
    Gui::Coord absCoord(event.coord());
    absCoord += absoluteBoundary().minCorner();

    MexLine3d cursorLine = cameraThroughCursorLine(absCoord, 1000.0);

    // Dispatch the click
    dispatchCursor(cursorLine, false, event.isCtrlPressed(), event.isShiftPressed(), event.isAltPressed());

    // Do special right hand mouse click funtionality if right hand mouse button is pressed
    // and above functions were not processed.
    if (event.rightButton() == Gui::PRESSED)
    {
        doMouseRightClickCameraMotion(event);
    }
}

void MachWorldViewWindow::doMouseRightClickCameraMotion(const GuiMouseEvent& event)
{
    Gui::Coord clickCoord = pInGameScreen_->rightClickMousePos();
    // Make original right click pos relative to world view window
    clickCoord.x(clickCoord.x() - absoluteBoundary().minCorner().x());
    clickCoord.y(clickCoord.y() - absoluteBoundary().minCorner().y());

    MexLine3d line(MexPoint3d(clickCoord.x(), clickCoord.y(), 0), MexPoint3d(event.coord().x(), event.coord().y(), 0));

    // Has mouse moved far enough away from original position to perform camera
    // motion. Must have moved at least 10 pixels.
    if (line.length() > 10)
    {
        MexLine3d hLine(MexPoint3d(clickCoord.x(), 0, 0), MexPoint3d(event.coord().x(), 0, 0));
        MexLine3d vLine(MexPoint3d(0, clickCoord.y(), 0), MexPoint3d(0, event.coord().y(), 0));

        if (hLine.length() > vLine.length())
        {
            if (event.coord().x() > clickCoord.x())
            {
                pCameras_->scroll(MachCameras::RIGHT, event);
            }
            else
            {
                pCameras_->scroll(MachCameras::LEFT, event);
            }
        }
        else
        {
            if (event.coord().y() > clickCoord.y())
            {
                pCameras_->scroll(MachCameras::DOWN, event);
            }
            else
            {
                pCameras_->scroll(MachCameras::UP, event);
            }
        }
    }
}

void MachWorldViewWindow::update()
{
    // Cause re-draw if mouse button pressed to show rubber band.
    if (mouseButtonPressed_)
    {
        // Check that mouse button is still pressed.
        if (DevMouse::instance().leftButton())
        {
            // Check current camera
            RubberBandCamera camera;
            if (pCameras_->isZenithCameraActive())
                camera = ZENITH;
            else if (pCameras_->isGroundCameraActive())
                camera = GROUND;
            else
            {
                // Camera has changed to invalid camera
                mouseButtonPressed_ = false;
                rubberBanding_ = false;
                unhighlightActorsSelectedViaRubberBand();
                pCameras_->resumeMotion();
                return;
            }

            if (camera != rubberBandCamera_)
            {
                // Camera has changed midway through rubber banding. Cancel rubber banding.
                mouseButtonPressed_ = false;
                rubberBanding_ = false;
                unhighlightActorsSelectedViaRubberBand();
                pCameras_->resumeMotion();
                return;
            }

            if (rubberBandCamera_ == ZENITH)
            {
                // Work out new startRubberBand_ based on startRubberBand3DPos_. This takes account of the
                // fact that scrolling may have happened.

                // The viewport must be correctly set to correspond to this window before
                // we can use the RenDevice methods.
                pInGameScreen_->setWorldViewViewport();

                W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();
                const MexTransform3d& cameraTransform = sceneManager.currentCamera()->globalTransform();
                MexPoint3d posIn3DWorld = startRubberBand3DPos_;
                cameraTransform.transformInverse(&posIn3DWorld);
                startRubberBand_ = RenDevice::current()->cameraToScreen(posIn3DWorld);

                // Reset the viewport correctly for GUI drawing.  TBD: a save/restore or
                // push/pop idiom would be much more robust.
                pInGameScreen_->setGuiViewport();
            }

            endRubberBand_.x(DevMouse::instance().position().first);
            endRubberBand_.y(DevMouse::instance().position().second);

            if (! rubberBanding_)
            {
                // Check that start and end points are far enough apart to start rubber banding.
                // Note : used MexLine3d because there doesn't appear to be a "length" function
                // for MexLine2d.
                MexLine3d line(
                    MexPoint3d(startRubberBand_.x(), startRubberBand_.y(), 0),
                    MexPoint3d(endRubberBand_.x(), endRubberBand_.y(), 0));
                if (line.length() > 10)
                {
                    rubberBanding_ = true;
                    if (rubberBandCamera_ == GROUND)
                        pCameras_->freezeMotion(); // Scrolling is not allowed in Ground camera when rubber banding
                }
            }

            if (rubberBanding_)
            {
                // Select all actors inside rubber band. Must press ctrl to keep already selected items
                updateActorsSelectedViaRubberBand(true, DevKeyboard::instance().ctrlPressed());
            }
        }
        else
        {
            if (rubberBanding_)
            {
                // Select all actors inside rubber band. Must press ctrl to keep already selected items
                updateActorsSelectedViaRubberBand(false, DevKeyboard::instance().ctrlPressed());
                pCameras_->resumeMotion();
                rubberBanding_ = false;
            }
            mouseButtonPressed_ = false;
        }
    }
}

void MachWorldViewWindow::unhighlightActorsSelectedViaRubberBand()
{
    MachLogRaces& races = MachLogRaces::instance();

    for (ctl_vector<W4dId>::iterator iter = selectedEntities_.begin(); iter != selectedEntities_.end(); ++iter)
    {
        if (races.actorExists(*iter))
        {
            if (races.actor(*iter).selectionState() == MachLog::HIGHLIGHTED)
                races.actor(*iter).selectionState(MachLog::NOT_SELECTED);
        }
    }

    selectedEntities_.erase(selectedEntities_.begin(), selectedEntities_.end());
}

void MachWorldViewWindow::updateActorsSelectedViaRubberBand(
    bool highlightOnly,
    bool keepSelection,
    bool inclMachines /*= true*/,
    bool inclConstructions /*= true*/)
{
    PRE(inclMachines || inclConstructions);

    if (! highlightOnly)
    {
        // Unselect all currently selected item if keepSelection is not true
        if (! keepSelection)
        {
            pInGameScreen_->deselectAll();
        }
    }

    unhighlightActorsSelectedViaRubberBand();
    W4dBoundingBoxSelector::Entities entitiesInRubberBand = getActorsInRectangle(startRubberBand_, endRubberBand_);

    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.playerRace();

    // Actors which are going to be selected into the corral
    MachInGameScreen::Actors selectMachines_;
    MachInGameScreen::Actors selectCanAttackMachines_;
    MachInGameScreen::Actors selectConstructions_;
    MachInGameScreen::Actors selectCanAttackConstructions_;
    selectMachines_.reserve(20);
    selectConstructions_.reserve(10);
    selectCanAttackMachines_.reserve(20);
    selectCanAttackConstructions_.reserve(10);

    for (W4dBoundingBoxSelector::Entities::iterator iter = entitiesInRubberBand.begin();
         iter != entitiesInRubberBand.end();
         ++iter)
    {
        W4dEntity* pEntity = *iter;
        if (races.actorExists(pEntity->id()))
        {
            MachActor& actor = races.actor(pEntity->id());

            // Only select actors belonging to player
            if (actor.race() == playerRace && actor.selectableType() == MachLog::FULLY_SELECTABLE
                && ((inclMachines && actor.objectIsMachine()) || (inclConstructions && actor.objectIsConstruction())))
            {
                if (highlightOnly) // Just highlight the actors for now because the rubber-banding hasn't finished
                {
                    if (actor.selectionState() != MachLog::SELECTED)
                    {
                        actor.selectionState(MachLog::HIGHLIGHTED);
                        selectedEntities_.push_back(pEntity->id());
                    }
                }
                else // Select actors into corral
                {
                    if (actor.selectionState() != MachLog::SELECTED)
                    {
                        if (actor.objectIsMachine())
                        {
                            if (actor.objectIsCanAttack())
                            {
                                selectCanAttackMachines_.push_back(&actor);
                            }
                            else
                            {
                                selectMachines_.push_back(&actor);
                            }
                        }
                        else
                        {
                            if (actor.objectIsCanAttack())
                            {
                                selectCanAttackConstructions_.push_back(&actor);
                            }
                            else
                            {
                                selectConstructions_.push_back(&actor);
                            }
                        }
                    }
                }
            }
        }
    }

    if (! highlightOnly)
    {
        selectedEntities_.erase(selectedEntities_.begin(), selectedEntities_.end());

        // Put actors into corral.
        // Can attack machines at front of corral
        if (selectCanAttackMachines_.size() != 0)
        {
            pInGameScreen_->select(selectCanAttackMachines_);
        }
        // Civilian Machines
        if (selectMachines_.size() != 0)
        {
            pInGameScreen_->select(selectMachines_);
        }
        // Missile emplacements and other can attack constructions
        if (selectCanAttackConstructions_.size() != 0)
        {
            pInGameScreen_->select(selectCanAttackConstructions_);
        }
        // Constructions at end of corral.
        if (selectConstructions_.size() != 0)
        {
            pInGameScreen_->select(selectConstructions_);
        }
    }
}

// virtual
void MachWorldViewWindow::doDisplay()
{
    // Draw rubber band
    if (rubberBanding_)
    {
        // The viewport must be correctly set to correspond to this window before
        // we can use the RenDevice methods.
        pInGameScreen_->setWorldViewViewport();

        GuiPainter::instance().hollowRectangle(Gui::Box(startRubberBand_, endRubberBand_), Gui::GREEN(), 1);

        // Reset the viewport correctly for GUI drawing.  TBD: a save/restore or
        // push/pop idiom would be much more robust.
        pInGameScreen_->setGuiViewport();
    }

}

bool MachWorldViewWindow::rubberBandSelectionHappening() const
{
    return rubberBanding_;
}

void MachWorldViewWindow::loadGame()
{
}

void MachWorldViewWindow::unloadGame()
{
}

bool MachWorldViewWindow::processButtonEvent(const DevButtonEvent& buttonEvent)
{
    if (buttonEvent.action() == DevButtonEvent::RELEASE || rubberBanding_)
        return false;

    typedef DevKeyToCommand::CommandId CommandId;

    CommandId commandId;
    bool processed = pKeyTranslator_->translate(buttonEvent, &commandId);

    if (processed)
    {
        // Make rubber band that is same size as worldview window
        Gui::Box worldviewArea = pInGameScreen_->getWorldViewWindowVisibleArea();

        startRubberBand_ = worldviewArea.minCorner();
        ;
        endRubberBand_ = worldviewArea.maxCorner();

        switch (commandId)
        {
            case SELECT_ONSCREEN_MACHINES:
                updateActorsSelectedViaRubberBand(false, false, true, false);
                break;
            case SELECT_ONSCREEN_CONSTRUCTIONS:
                updateActorsSelectedViaRubberBand(false, false, false, true);
                break;
            case SELECT_ONSCREEN_MACHINES_KEEP_SEL:
                updateActorsSelectedViaRubberBand(false, true, true, false);
                break;
            case SELECT_ONSCREEN_CONSTRUCTIONS_KEEP_SEL:
                updateActorsSelectedViaRubberBand(false, true, false, true);
                break;
        }
    }

    return processed;
}

UtlId MachWorldViewWindow::highlightedActorId() const
{
    return highLightedActorId_;
}

bool MachWorldViewWindow::haveHighlightedActor() const
{
    return haveHighlightedActor_;
}

ctl_pvector<W4dEntity> MachWorldViewWindow::getActorsInRectangle(const Gui::Coord& from, const Gui::Coord& to) const
{
    // The viewport must be correctly set to correspond to this window before
    // we can use the RenDevice methods.
    pInGameScreen_->setWorldViewViewport();

    W4dBoundingBoxSelector::Entities entitiesInRubberBand;
    entitiesInRubberBand.reserve(20);

    // Make sure rubber band area is big enough to continue. A small rubber band area will result
    // in an ASSERT in W4dBoundingBoxSelector.
    if (Mathex::abs(from.x() - to.x()) > MexEpsilon::instance()
        && Mathex::abs(from.y() - to.y()) > MexEpsilon::instance())
    {
        W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();
        MexPoint3d rbPoint1 = sceneManager.pDevice()->screenToCamera(from);
        MexPoint3d rbPoint2 = sceneManager.pDevice()->screenToCamera(MexPoint2d(from.x(), to.y()));
        MexPoint3d rbPoint3 = sceneManager.pDevice()->screenToCamera(to);
        MexPoint3d rbPoint4 = sceneManager.pDevice()->screenToCamera(MexPoint2d(to.x(), from.y()));

        // Sort rubber band points into clockwise order
        if ((to.x() > from.x() && to.y() > from.y()) || (to.x() < from.x() && to.y() < from.y()))
        {
            std::swap(rbPoint2, rbPoint4);
        }

        // Select entities inside the bounding box
        MexQuad3d boundingBox(rbPoint1, rbPoint2, rbPoint3, rbPoint4);

        W4dBoundingBoxSelector selector(*sceneManager.currentCamera(), boundingBox);

        selector.clipAtFogDistance(pCameras_->isGroundCameraActive());

        entitiesInRubberBand = selector.selectedEntities();
    }

    // Reset the viewport correctly for GUI drawing.  TBD: a save/restore or
    // push/pop idiom would be much more robust.
    pInGameScreen_->setGuiViewport();

    return entitiesInRubberBand;
}

ctl_pvector<W4dEntity> MachWorldViewWindow::getEntitiesInView()
{
    Gui::Box worldviewArea = pInGameScreen_->getWorldViewWindowVisibleArea();
    Gui::Coord from = worldviewArea.minCorner();
    Gui::Coord to = worldviewArea.maxCorner();

    return getActorsInRectangle(from, to);
}

/* End WORLDVIE.CPP *************************************************/
