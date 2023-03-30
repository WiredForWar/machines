/*
 * I G C U R S 2 D . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/igcurs2d.hpp"
#include "machgui/ingame.hpp"
#include "machgui/gui.hpp"

#include "gui/gui.hpp"
#include "world4d/scenemgr.hpp"
#include "render/animcurs.hpp"
#include "render/device.hpp"
#include "render/display.hpp"
#include "render/surface.hpp"
#include "system/pathname.hpp"

#include <stdlib.h>

MachInGameCursors2d::MachInGameCursors2d(MachInGameScreen* pInGameScreen)
    : pMenuCursor_(nullptr)
    , pEnterBuildingCursor_(nullptr)
    , pAttackCursor_(nullptr)
    , pConstructCursor_(nullptr)
    , pLocateToCursor_(nullptr)
    , pPatrolCursor_(nullptr)
    , pDeployCursor_(nullptr)
    , pPickUpCursor_(nullptr)
    , pScavengeCursor_(nullptr)
    , pTransportCursor_(nullptr)
    , pEnterAPCCursor_(nullptr)
    , pFollowCursor_(nullptr)
    , pDropLandMineCursor_(nullptr)
    , pHealCursor_(nullptr)
    , pDeconstructCursor_(nullptr)
    , pRecycleCursor_(nullptr)
    , pRepairCursor_(nullptr)
    , pCaptureCursor_(nullptr)
    , pJoinConstructCursor_(nullptr)
    , pAssemblePointCursor_(nullptr)
    , pScrollWestCursor_(nullptr)
    , pScrollEastCursor_(nullptr)
    , pScrollNorthCursor_(nullptr)
    , pScrollSouthCursor_(nullptr)
    , pScrollNorthWestCursor_(nullptr)
    , pScrollNorthEastCursor_(nullptr)
    , pScrollSouthWestCursor_(nullptr)
    , pScrollSouthEastCursor_(nullptr)
    , pChooseConstructionCursor_(nullptr)
    , pTreacheryCursor_(nullptr)
    , currentType_(MachGui::MOVETO_CURSOR)
    , pInGameScreen_(pInGameScreen)
    , cursorSize_(LARGECURSORS)
{
    // Load the cursor bitmaps
    loadCursors();

    TEST_INVARIANT;
}

MachInGameCursors2d::~MachInGameCursors2d()
{
    _DELETE(pMenuCursor_);
    _DELETE(pMoveCursor_[LARGECURSORS]);
    _DELETE(pMoveCursor_[SMALLCURSORS]);
    _DELETE(pEnterBuildingCursor_);
    _DELETE(pNoEntryCursor_[LARGECURSORS]);
    _DELETE(pNoEntryCursor_[SMALLCURSORS]);
    _DELETE(pAttackCursor_);
    _DELETE(pConstructCursor_);
    _DELETE(pLocateToCursor_);
    _DELETE(pPatrolCursor_);
    _DELETE(pDeployCursor_);
    _DELETE(pPickUpCursor_);
    _DELETE(pScavengeCursor_);
    _DELETE(pTransportCursor_);
    _DELETE(pEnterAPCCursor_);
    _DELETE(pFollowCursor_);
    _DELETE(pSelectCursor_[LARGECURSORS]);
    _DELETE(pSelectCursor_[SMALLCURSORS]);
    _DELETE(pDropLandMineCursor_);
    _DELETE(pHealCursor_);
    _DELETE(pDeconstructCursor_);
    _DELETE(pRecycleCursor_);
    _DELETE(pRepairCursor_);
    _DELETE(pCaptureCursor_);
    _DELETE(pJoinConstructCursor_);
    _DELETE(pAssemblePointCursor_);
    _DELETE(pScrollSouthCursor_);
    _DELETE(pScrollNorthCursor_);
    _DELETE(pScrollEastCursor_);
    _DELETE(pScrollWestCursor_);
    _DELETE(pScrollNorthWestCursor_);
    _DELETE(pScrollNorthEastCursor_);
    _DELETE(pScrollSouthWestCursor_);
    _DELETE(pScrollSouthEastCursor_);
    _DELETE(pChooseConstructionCursor_);
    _DELETE(pTreacheryCursor_);

    _DELETE(pIonAttackCursor_);
    _DELETE(pNukeAttackCursor_);

    TEST_INVARIANT;
}

void MachInGameCursors2d::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachInGameCursors2d& t)
{

    o << "MachInGameCursors2d " << (void*)&t << " start" << std::endl;
    o << "MachInGameCursors2d " << (void*)&t << " end" << std::endl;

    return o;
}

void MachInGameCursors2d::loadCursors()
{
    // Get the back buffer
    const GuiBitmap& backBuffer = Gui::backBuffer();

    pMenuCursor_ = loadCursor(MachCursorSpec("large/arrow", 1, { 0, 0 }));
    pEnterBuildingCursor_ = loadCursor(MachCursorSpec("large/enter", 6));
    pAttackCursor_ = loadCursor(MachCursorSpec("large/attack", 2));
    pIonAttackCursor_ = loadCursor(MachCursorSpec("large/ioncan", 10, { 27, 38 }));
    pConstructCursor_ = loadCursor(MachCursorSpec("large/arrow", 1, { 0, 0 }));
    pLocateToCursor_ = loadCursor(MachCursorSpec("large/locat", 5, { 8, 14 }));
    pPatrolCursor_ = loadCursor(MachCursorSpec("large/patrol", 6));
    pDeployCursor_ = loadCursor(MachCursorSpec("large/deploy", 2));
    pPickUpCursor_ = loadCursor(MachCursorSpec("large/pickup", 2, { 8, 8 }, 7.5));
    pScavengeCursor_ = loadCursor(MachCursorSpec("large/scav", 10, { 8, 15 }));
    pTransportCursor_ = loadCursor(MachCursorSpec("large/putdn", 2, { 8, 8 }, 7.5));
    pEnterAPCCursor_ = loadCursor(MachCursorSpec("large/entapc", 6));
    pFollowCursor_ = loadCursor(MachCursorSpec("large/follow", 4));
    pDropLandMineCursor_ = loadCursor(MachCursorSpec("large/landmin", 2));
    pHealCursor_ = loadCursor(MachCursorSpec("large/heal", 6));
    pDeconstructCursor_ = loadCursor(MachCursorSpec("large/deconst", 9));
    pRecycleCursor_ = loadCursor(MachCursorSpec("large/recycle", 4));
    pRepairCursor_ = loadCursor(MachCursorSpec("large/repair", 1));
    pCaptureCursor_ = loadCursor(MachCursorSpec("large/capture", 8));
    pJoinConstructCursor_ = loadCursor(MachCursorSpec("large/jconst", 11));
    pAssemblePointCursor_ = loadCursor(MachCursorSpec("large/assembl", 4));
    pNukeAttackCursor_ = loadCursor(MachCursorSpec("large/nukattk", 5));
    pScrollSouthCursor_ = loadCursor(MachCursorSpec("large/scrls", 1, { 6, 13 }));
    pScrollNorthCursor_ = loadCursor(MachCursorSpec("large/scrln", 1, { 6, 0 }));
    pScrollEastCursor_ = loadCursor(MachCursorSpec("large/scrle", 1, { 13, 6 }));
    pScrollWestCursor_ = loadCursor(MachCursorSpec("large/scrlw", 1, { 0, 6 }));
    pScrollNorthWestCursor_ = loadCursor(MachCursorSpec("large/scrlnw", 1, { 0, 0 }));
    pScrollNorthEastCursor_ = loadCursor(MachCursorSpec("large/scrlne", 1, { 14, 0 }));
    pScrollSouthWestCursor_ = loadCursor(MachCursorSpec("large/scrlsw", 1, { 0, 14 }));
    pScrollSouthEastCursor_ = loadCursor(MachCursorSpec("large/scrlse", 1, { 14, 14 }));
    pChooseConstructionCursor_ = loadCursor(MachCursorSpec("large/const", 11));
    pTreacheryCursor_ = loadCursor(MachCursorSpec("large/treach", 5, { 4, 4 }));

    pSelectCursor_[LARGECURSORS] = loadCursor(MachCursorSpec("large/select", 1));
    pSelectCursor_[SMALLCURSORS] = loadCursor(MachCursorSpec("small/select", 1));
    pNoEntryCursor_[LARGECURSORS] = loadCursor(MachCursorSpec("large/noentry", 1));
    pNoEntryCursor_[SMALLCURSORS] = loadCursor(MachCursorSpec("small/noentry", 1));
    pMoveCursor_[LARGECURSORS] = loadCursor(MachCursorSpec("large/move", 4));
    pMoveCursor_[SMALLCURSORS] = loadCursor(MachCursorSpec("small/move", 4));
}

RenCursor2d* MachInGameCursors2d::loadCursor(const MachCursorSpec& cursorSpec)
{
    int nFrames = cursorSpec.getFramesNumber();
    double fps = cursorSpec.getFps();
    std::string baseName = cursorSpec.getName();
    MexPoint2d origin = cursorSpec.origin();

    ASSERT(nFrames < 100, "No more than 99 frames allowed for animated cursors - this is the current arbitrary limit.");

    if (fps == 0)
    {
        constexpr double defaultFps = 10;
        fps = defaultFps;
    }

    // Get the back buffer
    const GuiBitmap& backBuffer = Gui::backBuffer();

    // Create the main cursor
    RenAnimCursor2d* pCursor = _NEW(RenAnimCursor2d);

    // Create and add the surface for each frame
    char buffer[3];
    std::string cursorDir = "gui/cursor/";
    std::string extention;

    std::string basePath = cursorDir + baseName;
    {
        std::string withFrames = nFrames > 1 ? basePath + "1" : basePath;
        extention = MachGui::getScaledImagePath(std::string());

        if (SysPathName::existsAsFile(withFrames + extention))
        {
            origin *= MachGui::uiScaleFactor();
        }
        else
        {
            // Fallback
            extention = ".bmp";

            ASSERT(SysPathName::existsAsFile(withFrames + extention), "Unable to load a cursor: file not found");
        }
        if (nFrames > 1)
        {
            sprintf(buffer, "%d", nFrames * 2);
            if (SysPathName::existsAsFile(basePath + buffer + extention))
            {
                // Twice many frames
                fps *= 2;
                nFrames *= 2;
            }
        }
    }

    for (int i = 0; i < nFrames; ++i)
    {
        // Construct the full name
        std::string name = basePath;
        if (nFrames > 1)
        {
            int frameNumber = i + 1;
            //          name += itoa( frameNumber, buffer, 10 );
            sprintf(buffer, "%d", frameNumber);
            name += buffer;
        }

        name += extention;
        //++index;

        // Create the surface
        GuiBitmap surface = Gui::bitmap(SysPathName(name));
        surface.enableColourKeying();

        if (i == 0)
        {
            if (!cursorSpec.hasCustomOrigin())
            {
                // Get the surface dimensions
                origin = MexPoint2d(surface.width(), surface.height()) * 0.5;
            }
        }

        // Add to the cursor
        pCursor->addFrame(surface);
    }

    // Set the origin
    pCursor->origin(origin.x(), origin.y());

    // set the frame rate
    pCursor->targetFrequency(fps);

    return pCursor;
}

void MachInGameCursors2d::forceCursor(MachGui::Cursor2dType type)
{
    // decide which cursor to use
    RenCursor2d* pNewCursor = nullptr;

    switch (type)
    {
        case MachGui::MENU_CURSOR:
            pNewCursor = pMenuCursor_;
            break;
        case MachGui::INVALID_CURSOR:
            pNewCursor = pNoEntryCursor_[cursorSize_];
            break;
        case MachGui::MOVETO_CURSOR:
            pNewCursor = pMoveCursor_[cursorSize_];
            break;
        case MachGui::ATTACK_CURSOR:
            pNewCursor = pAttackCursor_;
            break;
        case MachGui::ION_ATTACK_CURSOR:
            pNewCursor = pIonAttackCursor_;
            break;
        case MachGui::NUKE_ATTACK_CURSOR:
            pNewCursor = pNukeAttackCursor_;
            break;
        case MachGui::CONSTRUCT_CURSOR:
            pNewCursor = pConstructCursor_;
            break;
        case MachGui::LOCATETO_CURSOR:
            pNewCursor = pLocateToCursor_;
            break;
        case MachGui::PATROL_CURSOR:
            pNewCursor = pPatrolCursor_;
            break;
        case MachGui::DEPLOY_CURSOR:
            pNewCursor = pDeployCursor_;
            break;
        case MachGui::PICKUP_CURSOR:
            pNewCursor = pPickUpCursor_;
            break;
        case MachGui::SCAVENGE_CURSOR:
            pNewCursor = pScavengeCursor_;
            break;
        case MachGui::TRANSPORT_CURSOR:
            pNewCursor = pTransportCursor_;
            break;
        case MachGui::ENTER_BUILDING_CURSOR:
            pNewCursor = pEnterBuildingCursor_;
            break;
        case MachGui::ENTER_APC_CURSOR:
            pNewCursor = pEnterAPCCursor_;
            break;
        case MachGui::FOLLOW_CURSOR:
            pNewCursor = pFollowCursor_;
            break;
        case MachGui::SELECT_CURSOR:
            pNewCursor = pSelectCursor_[cursorSize_];
            break;
        case MachGui::DROPLANDMINE_CURSOR:
            pNewCursor = pDropLandMineCursor_;
            break;
        case MachGui::HEAL_CURSOR:
            pNewCursor = pHealCursor_;
            break;
        case MachGui::DECONSTRUCT_CURSOR:
            pNewCursor = pDeconstructCursor_;
            break;
        case MachGui::RECYCLE_CURSOR:
            pNewCursor = pRecycleCursor_;
            break;
        case MachGui::REPAIR_CURSOR:
            pNewCursor = pRepairCursor_;
            break;
        case MachGui::CAPTURE_CURSOR:
            pNewCursor = pCaptureCursor_;
            break;
        case MachGui::JOINCONSTRUCT_CURSOR:
            pNewCursor = pJoinConstructCursor_;
            break;
        case MachGui::ASSEMBLEPOINT_CURSOR:
            pNewCursor = pAssemblePointCursor_;
            break;
        case MachGui::SCROLL_W_CURSOR:
            pNewCursor = pScrollWestCursor_;
            break;
        case MachGui::SCROLL_E_CURSOR:
            pNewCursor = pScrollEastCursor_;
            break;
        case MachGui::SCROLL_N_CURSOR:
            pNewCursor = pScrollNorthCursor_;
            break;
        case MachGui::SCROLL_S_CURSOR:
            pNewCursor = pScrollSouthCursor_;
            break;
        case MachGui::SCROLL_NW_CURSOR:
            pNewCursor = pScrollNorthWestCursor_;
            break;
        case MachGui::SCROLL_NE_CURSOR:
            pNewCursor = pScrollNorthEastCursor_;
            break;
        case MachGui::SCROLL_SE_CURSOR:
            pNewCursor = pScrollSouthEastCursor_;
            break;
        case MachGui::SCROLL_SW_CURSOR:
            pNewCursor = pScrollSouthWestCursor_;
            break;
        case MachGui::CHOOSE_CONST_CURSOR:
            pNewCursor = pChooseConstructionCursor_;
            break;
        case MachGui::TREACHERY_CURSOR:
            pNewCursor = pTreacheryCursor_;
            break;
    }

    // Get the display handle
    W4dSceneManager& sceneManager = pInGameScreen_->sceneManager();
    RenDisplay* pDisplay = sceneManager.pDevice()->display();

    // Use the selected cursor
    if (pNewCursor == nullptr)
        pNewCursor = pMenuCursor_;

    pDisplay->useCursor(pNewCursor);

    // Store which one we are using
    currentType_ = type;
}

void MachInGameCursors2d::cursor(MachGui::Cursor2dType type)
{
    // Check not already using it
    if (type != currentType_)
    {
        forceCursor(type);
    }
}

MachGui::Cursor2dType MachInGameCursors2d::type() const
{
    return currentType_;
}

void MachInGameCursors2d::cursorSize(CursorSize curSize)
{
    cursorSize_ = curSize;
}

/* End IGCURS2D.CPP *************************************************/
