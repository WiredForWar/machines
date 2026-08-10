/*
 * F S T P E R S N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiFirstPerson

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_FSTPERSN_HPP
#define _MACHGUI_FSTPERSN_HPP

#include "base/base.hpp"
#include "gui/Root.hpp"
#include "world4d/Subject/Observer.hpp"
#include "world4d/Subject/Subject.hpp"
#include "machphys/machphys.hpp"
#include "device/KeyToCommandTranslator.hpp"
#include "gui/gui.hpp"
#include "mathex/Point3d.hpp"
#include "mathex/Random.hpp"
#include "phys/phys.hpp"

class W4dSceneManager;
class W4dRoot;
class GuiMouseEvent;
class MachActor;
class MachInGameScreen;
class W4dDomain;
class MachGuiAnimation;
class MachGuiFPCommand;
class MachGuiInGameChatMessagesDisplay;
class MachGuiPausedImage;
class MachGuiRadar;
class MachLog1stPersonHandler;

class MachGuiFirstPerson
    : public GuiRoot
    , public W4dObserver
// Canonical form revoked
{
public:
    MachGuiFirstPerson(W4dSceneManager* pSceneManager, W4dRoot* pRoot, MachInGameScreen*);
    ~MachGuiFirstPerson() override;

    void CLASS_INVARIANT;

    bool switchToMenus() const;
    bool switchToInGameRequested() const;
    void resetSwitchToMenus();
    void resetSwitchToInGame();
    void switchToInGame();

    // Set the actor that we are controlling
    void setActor(MachActor*);

    // Set MachActor* to NULL.
    void resetActor();

    // Checks the actor type to make sure first person is an option.
    bool okayToSwitchTo1stPerson();

    // Set up viewport for first person stuff.
    void setFirstPerson3DViewport();

    // Called after the app has been given windows focus ( WM_APPACTIVATE ).
    void activate();

    // Called when the ingame reolution has changed
    void resolutionChange();

protected:
    void displayCompass();
    void displayHealthArmour();
    void displayWeapons();
    bool displayWeapon(
        MachPhys::Mounting,
        const Gui::Coord& topLeft,
        const GuiBitmap& weaponBmp,
        double changeEndTime,
        int& weaponFrame);
    void updateWeaponAnimEndTime(double&);
    void loadWeaponBmps();
    GuiBitmap getWeaponBmp(MachPhys::WeaponType);
    void doWeaponSelect();

    // Load bitmaps in preperation for rendering. Too slow reading them off disk.
    void loadBitmaps();

    // Unload bitmaps when leaving first person to conserve memory usage.
    void unloadBitmaps();

    void doDisplay() override;
    void update() override;
    void doBecomeRoot() override;
    void doBecomeNotRoot() override;
    bool doHandleRightClickEvent(const GuiMouseEvent&) override;
    bool doHandleKeyEvent(const GuiKeyEvent&) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;

    // Inherited from W4dObserver
    // true iff this observer is to exist in this subject's list of observers
    // following this call. This will typically be implemented using double dispatch.
    // The clientData is of interest only if event == CLIENT_SPECIFIC.Interpretation
    // is client defined.
    bool beNotified(W4dSubject*, W4dSubject::NotificationEvent, int clientData) override;

    // Informs observer that an observed domain is being deleted.
    // This observer need not call the W4dDomain::detach() method - this
    // will be done automatically.
    void domainDeleted(W4dDomain*) override;

    enum Command
    {
        FOWARD,
        BACKWARD,
        TURNLEFT,
        TURNRIGHT,
        TURNLEFTFAST,
        TURNRIGHTFAST,
        FIRE,
        CENTREHEAD,
        WEAPONSELECT,
        TURNHEADLEFT,
        TURNHEADRIGHT,
        TURNHEADLEFTFAST,
        TURNHEADRIGHTFAST,
        LOOKUP,
        LOOKDOWN,
        LOOKUPFAST,
        LOOKDOWNFAST,
        COMMAND_SELECT_NEXT,
        COMMAND_SELECT_PREV,
        COMMAND_ORDER_ATTACK,
        COMMAND_ORDER_FOLLOW,
        COMMAND_ORDER_MOVE,
        NUM_COMMANDS /*must come last*/
    };

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiFirstPerson& t);

    void embodyActor();
    void exitActor();

    // Sort out the first person camera attachment to the 1st person entity
    void attachCamera();
    void detachCamera();

    // revoked
    MachGuiFirstPerson(const MachGuiFirstPerson&) = delete;
    MachGuiFirstPerson& operator=(const MachGuiFirstPerson&) = delete;

    // Data members...
    bool switchToMenus_{};
    bool switchToInGame_{};
    MachActor* pActor_{};
    MachInGameScreen* pInGameScreen_{};
    bool inFirstPerson_{};
    W4dSceneManager* pSceneManager_{};
    int borderHeight_;
    int lastBorderHeight_;
    DevKeyToCommandTranslator* pKeyTranslator_;
    DevKeyToCommandTranslator::CommandList commandList_;
    MachLog1stPersonHandler* pLogHandler_{}; // Handles 1st person commands - makes things happen in game
    MexPoint3d targetPoint_; // The point currently to be aimed at (global coords)
    MachGuiAnimation* pAttackCursor_{}; // Attack cross-hair
    MachGuiAnimation* pNormalCursor_{}; // Nothing to target cursor
    MachGuiAnimation* pMissCursor_{}; // Targeted on actor but weapons cannot tilt
    MachGuiAnimation* pStartCursor_{}; // When machine is first embodied, cursor expands.
    bool switchBackToGroundCamera_ = true; // Camera to switch back to when leaving 1st person
    MachActor* pTargetActor_{};
    GuiBitmap compassBmp_;
    GuiBitmap healthBmp_;
    GuiBitmap armourBmp_;
    MachGuiRadar* pRadar_{};
    int borderDrawCount_;
    GuiBitmap leftWeaponBmp_;
    GuiBitmap rightWeaponBmp_;
    GuiBitmap topWeaponBmp_;
    GuiBitmap weaponChargeBmp_;
    GuiBitmap weaponBackgroundBmp_;
    int weaponSelectIndex_;
    bool justEnteredFirstPerson_;
    GuiBitmap weaponStartupFrames_[10];
    double leftWeaponChangeEndTime_;
    double rightWeaponChangeEndTime_;
    double topWeaponChangeEndTime_;
    int leftWeaponPos_;
    int rightWeaponPos_;
    int topWeaponPos_;
    bool resolutionChanged_ = true;
    bool isDead_;
    PhysAbsoluteTime timeOfDeath_;
    MachGuiInGameChatMessagesDisplay* pChatMessageDisplay_{};
    bool rightMouseButtonHeadTurningUsed_{};
    double lastRightClickTime_{}; // Used for checking for right mouse button double click
    double timeWeaponsFired_;
    MachGuiPausedImage* pPausedImage_{};
    bool reverseUpDownKeys_;
    bool reverseUpDownMouse_;
    MexBasicRandom hitInterferenceRandom_;
    bool machineNVGOn_{};
    double startupTimer_;
    bool finishedStartupSequence_{};
    bool isHitInterferenceOn_;
    double hitInterferenceEndTime_;
    int frameNumber_;

    // FP Command
    MachGuiFPCommand* pCommandWidget_{};
    int64_t commandSquadIndex_ = -1L;
    double timeSquadIndexChanged_{};
};

#endif

/* End FSTPERSN.HPP *************************************************/
