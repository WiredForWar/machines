/*
 * I G C U R S 2 D . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachInGameCursors2d

    Supports the loading and switching between all the in-game screen 2d cursors
*/

#ifndef _MACHGUI_IGCURS2D_HPP
#define _MACHGUI_IGCURS2D_HPP

#include "base/base.hpp"
#include "mathex/point2d.hpp"
#include "stdlib/string.hpp"
#include "machgui/gui.hpp"

// Forward refs
class RenCursor2d;
class MachInGameScreen;

class MachCursorSpec
{
public:
    MachCursorSpec(const std::string& name, int nFrames)
        : name_(name)
        , nFrames_(nFrames)
    {
    }

    MachCursorSpec(const std::string& name, int nFrames, const MexPoint2d& origin, double fps = 0)
        : MachCursorSpec(name, nFrames)
    {
        customOrigin_ = true;
        origin_ = origin;
        fps_ = fps;
    }

    std::string getName() const { return name_; }

    int getFramesNumber() const { return nFrames_; }

    double getFps() const { return fps_; }

    bool hasCustomOrigin() const { return customOrigin_; }

    MexPoint2d origin() const { return origin_; }

private:
    std::string name_;
    int nFrames_ = 0;
    double fps_ = 0;
    MexPoint2d origin_;
    bool customOrigin_ = false;
};

// orthodox canonical (revoked)
class MachInGameCursors2d
{
public:
    // ctor.
    MachInGameCursors2d(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachInGameCursors2d();

    static RenCursor2d* loadCursor(const MachCursorSpec& cursorSpec);

    // Get/Select the appropriate current cursor
    void cursor(MachGui::Cursor2dType type);
    MachGui::Cursor2dType type() const;

    // Same as "cursor" only there is no initial check that we are
    // selecting the same cursor again. This should be used if a
    // class other that MachInGameScreen has at some point changed the
    // cursor.
    void forceCursor(MachGui::Cursor2dType type);

    // Change between small and large cursors ( small used on continent map )
    enum CursorSize
    {
        LARGECURSORS,
        SMALLCURSORS
    };
    void cursorSize(CursorSize);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachInGameCursors2d& t);

private:
    // Operations deliberately revoked
    MachInGameCursors2d(const MachInGameCursors2d&);
    MachInGameCursors2d& operator=(const MachInGameCursors2d&);
    bool operator==(const MachInGameCursors2d&);

    // Load up all the cursors
    void loadCursors();

    // Data members
    RenCursor2d* pMenuCursor_; // The one over the menus etc
    RenCursor2d* pMoveCursor_[2]; // Move to terrain location
    RenCursor2d* pEnterBuildingCursor_; // Enter a building
    RenCursor2d* pNoEntryCursor_[2]; // Invalid pick
    RenCursor2d* pAttackCursor_; // Attack something with non-ion and non-nuke weapons
    RenCursor2d* pIonAttackCursor_; // Attack something with an ion beam
    RenCursor2d* pNukeAttackCursor_; // Attack something with a nuclear strike
    RenCursor2d* pConstructCursor_; // Construct a building
    RenCursor2d* pLocateToCursor_; // Locate along path
    RenCursor2d* pPatrolCursor_; // Patrol up and down path
    RenCursor2d* pDeployCursor_; // APC deploy contents
    RenCursor2d* pPickUpCursor_; // Collect ore/BMUs
    RenCursor2d* pScavengeCursor_; // Scavenge debris
    RenCursor2d* pTransportCursor_; // Deliver ore/debris to smelter
    RenCursor2d* pEnterAPCCursor_; // Enter an APC
    RenCursor2d* pFollowCursor_; // Enter an APC
    RenCursor2d* pSelectCursor_[2]; // Select the entity
    RenCursor2d* pDropLandMineCursor_; // drop a land mine
    RenCursor2d* pHealCursor_; // Heal a machine cursor
    RenCursor2d* pDeconstructCursor_; // Deconstruct a building cursor
    RenCursor2d* pRecycleCursor_; // Recycle a machine cursor
    RenCursor2d* pRepairCursor_; // Repair cursor
    RenCursor2d* pCaptureCursor_; // Capture a building cursor
    RenCursor2d* pJoinConstructCursor_; // Join in on a construction cursor
    RenCursor2d* pAssemblePointCursor_; // Assemble point cursor
    RenCursor2d* pScrollWestCursor_; // When camera is scrolling left
    RenCursor2d* pScrollEastCursor_; // When camera is scrolling right
    RenCursor2d* pScrollNorthCursor_; // When camera is scrolling up
    RenCursor2d* pScrollSouthCursor_; // When camera is scrolling down
    RenCursor2d* pScrollNorthWestCursor_; // When camera is scrolling up and left
    RenCursor2d* pScrollNorthEastCursor_; // When camera is scrolling up and right
    RenCursor2d* pScrollSouthWestCursor_; // When camera is scrolling down and left
    RenCursor2d* pScrollSouthEastCursor_; // When camera is scrolling down and right
    RenCursor2d* pChooseConstructionCursor_; // When construct has been selected but no construction
    RenCursor2d* pTreacheryCursor_; // Target an enemy machine to be treacheried
    MachGui::Cursor2dType currentType_; // The current cursor
    MachInGameScreen* pInGameScreen_; // The gui root
    CursorSize cursorSize_; // Cursor size used ( some cursors come in two sizes )
};

#endif

/* End IGCURS2D.HPP *************************************************/
