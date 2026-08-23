/*
 * I N G A M E . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachInGameScreen

    Top level GuiDisplayable for the in game screen.
*/

#ifndef _MACHGUI_INGAME_HPP
#define _MACHGUI_INGAME_HPP

#include "base/base.hpp"
#include "world4d/Subject/Observer.hpp"
#include "gui/Root.hpp"
#include "gui/ResolvedUiString.hpp"
#include "machgui/gui.hpp"
#include "machgui/CursorSpec.hpp"
#include "ctl/PtrVector.hpp"
#include "phys/phys.hpp"
#include "utility/CallbackHandle.hpp"
#include "world4d/Scene/Domain.hpp"

#include <memory>
#include <optional>
#include <string>

// Forward refs
class IProgressReporter;
class MachGuiBmuButton;
class MachContinentMap;
class MachMachinesIcon;
class MachConstructionsIcon;
class MachSquadronIcon;
class MachGuiCorralSingleIcon;
class MachGuiMachineNavigator;
class MachGuiConstructionNavigator;
class MachCommandIcons;
class MachSmallCommandIcons;
class MachConstructMenu;
class MachBuildMenu;
class MachHWResearchMenu;
class MachProductionBank;
class MachHWResearchBank;
class MachGuiSquadronBank;
class MachInGameCursors2d;
class MachGuiFirstPerson;
class MachGuiControlPanelAddOn;
class MachGuiControlPanel;
class MachGuiMapArea;
class MachGuiCameraScrollArea;
class MachGuiInGameChatMessagesDisplay;
class MachGuiCommand;
class MachWorldViewWindow;
class MachGuiCorral;
class MachPromptText;
class MachGuiDefconCommand;
class MachGuiSelfDestructCommand;
class MachGuiIonAttackCommand;
class MachGuiNukeAttackCommand;
class MachCameras;
class W4dSceneManager;
class W4dRoot;
class MachActor;
class MachGuiConsoleDropDown;
class MachGuiControlPanel;
template <class T> class ctl_pvector;

namespace System
{

class IConsole;

} // namespace System

// orthodox canonical (revoked)
class MachInGameScreen
    : public GuiRoot
    , public W4dObserver
{
public:
    // The scene manager for the 3d window is pSceneManager
    MachInGameScreen(W4dSceneManager* pSceneManager, W4dRoot* pRoot, IProgressReporter* pReporter);

    ~MachInGameScreen() override;

    void setConsole(System::IConsole* console);
    void setConsoleDropDown(MachGuiConsoleDropDown* pDropDown);
    void reattachConsoleDropDown();

    // Create the cameras, initialises button display values and map
    void loadGame(const std::string& planet, std::optional<PerIstream *> savedStream = std::nullopt);

    // Save info
    void saveGame(PerOstream& outStream);

    // Delete the game's cameras
    void unloadGame();

    // Export components
    MachGuiCorral& corral();
    MachWorldViewWindow& worldViewWindow();
    MachPromptText& promptTextWindow();

    // True if component is to be displayed
    bool isCorralVisible() const;
    bool isCorralSingleIconVisible() const;
    bool isCommandIconsVisible() const;
    bool isSmallCommandIconsVisible() const;
    bool isMachineNavigationVisible() const;
    bool isConstructionNavigationVisible() const;
    bool isSquadronBankVisible() const;

    using Actors = ctl_pvector<MachActor>;
    using Commands = std::vector<std::unique_ptr<MachGuiCommand>>;

    // The collection of selected actors
    const Actors& selectedActors() const;

    // True if actor is currently in the selected set
    bool isSelected(MachActor& actor) const;

    // Add pActor to the collection of selected actors
    void select(MachActor* pActor);
    // PRE( not isSelected( *pActor ) );

    // Add Actors to the collection of selected actors
    void select(const Actors& actors);

    // Remove pActor from the collection of selected actors
    void deselect(MachActor* pActor);
    // PRE( isSelected( *pActor ) );

    // Remove Actors from the collection of selected actors
    void deselect(const Actors& actors);

    // Deselect all currently selected actors
    void deselectAll();

    Actors getVisibleActors() const;

    // The list of all available commands
    const Commands& allCommands() const;

    // The default command
    MachGuiCommand& defaultCommand() const;

    // Make the default command active, cancelling any other active command
    void activateDefaultCommand();

    // Set/get the active command.
    void activeCommand(const MachGuiCommand& command);
    MachGuiCommand& activeCommand() const;

    // Flags any active command to be cancelled on next update.
    // This deferred cancellation is udeful for situations in which the
    // need to cancel the active command arises potentially when methods of
    // the command are on the call stack.
    void cancelActiveCommand();

    // Set/get the prompt text related to cursor position
    void setCursorPromptTextToPos(MexPoint2d point);
    void setCursorPromptText(const ResolvedUiString& prompt);
    void setCursorPromptText(const ResolvedUiString& prompt, bool restartScroll);
    const std::string& cursorPromptText() const;

    // Clear any prompt text associated with the cursor position
    void clearCursorPromptText();

    // Set/get the prompt text related to current command state
    void commandPromptText(const ResolvedUiString& prompt);
    const std::string& commandPromptText() const;

    // Clear any prompt text associated with the current command state
    void clearCommandPromptText();

    // set the current cursor
    void cursor2d(MachGui::Cursor2dType type);
    void cursor2d(MachGui::Cursor2dType type, MachInGameCursors2d::CursorSize curSize);

    // Update the command bank, if displayed
    void updateCommandIcons();

    // Access top context settings. It would be unusual for clients to use these methods.
    // This class is responsible for toggling display/activity of components.
    bool isMainMenuContext() const;
    bool isSquadronContext() const;
    bool isConstructCommandContext() const;
    bool isBuildCommandContext() const;
    bool isHardwareResearchContext() const;
    bool isSoftwareResearchContext() const;
    bool isSingleFactoryContext() const;
    bool isMachineNavigationContext() const;
    bool isConstructionNavigationContext() const;

    // Enter contexts for the menus
    void resetContext(); // Clear all context flags
    void mainMenuContext();
    void squadronContext();
    void constructCommandContext();
    void buildCommandContext();
    void hardwareResearchContext();
    void softwareResearchContext();
    void singleFactoryContext();
    void constructionNavigationContext();
    void machineNavigationContext();
    void mainMenuOrSingleFactoryContext();

    // Set the control panel context. Force update if you are switching to the same context
    // but want the code to re-evaluate the position of gui controls.
    void currentContext(MachGui::ControlPanelContext newContext, bool forceUpdate = false);

    // The current control panel context
    MachGui::ControlPanelContext currentContext() const;

    // The 3d window scene manager
    W4dSceneManager& sceneManager() const;

    // Set/get the cursor filter to be used in checking for objects under the cursor.
    // This filter has the same meaning as the filter argument to W4dDomain::findNearerEntity().
    void setCursorFilter(int filter);
    int cursorFilter() const;

    // Inherited from GuiRoot
    // Perform any per-frame processing after event processing
    void update() override;

    // Bits of the update method whose ordering in the loop cycle does not matter.
    // This CPU work can be pipelined between the 3D and 2D rendering.
    void asynchronousUpdate();

    void updateCameras();

    // Indicate if next command button pressed should be applied to a squad
    bool applyCommandToSquadron();
    void applyCommandToSquadron(bool);

    // Get at the different cameras viewing the WorldViewWindow
    MachCameras* cameras();
    // PRE( pCameras_ );

    // Highlight the actor ( i.e. surround with yellow box in world view window ), and
    // display information about this actor in corral ( only if no other actors are selected ).
    void highlightActor(MachActor*);
    void removeHighlightedActor();
    void displayActorPromptText(const MachActor* pActor);

    // Set the viewport to the fullscreen or to the world-view window, respectivley.
    void setGuiViewport();
    void setWorldViewViewport();

    // If true then the world view window is currently involved in a rubber band selection
    bool rubberBandSelectionHappening() const;

    enum GameState
    {
        PLAYING,
        WON,
        LOST
    };

    // Returns if game has been won/lost or is still being played. There is a slight
    // delay in this returning the actual gameState to enable victory/defeat music to be played.
    GameState gameState() const;
    // Same as above only no delay.
    GameState actualGameState() const;

    // Called after a WM_ACTIVATE message to ensure that the screen is re-drawn.
    void activate();

    MachGuiDefconCommand* defconCommand();
    MachGuiSelfDestructCommand* selfDestructCommand();
    MachGuiIonAttackCommand* ionAttackCommand();
    MachGuiNukeAttackCommand* nukeAttackCommand();

    // Switch FOW on or off. When playing skirmish levels or multiplayer
    // this is a setable option
    void fogOfWarOn(bool fog);
    bool fogOfWarOn() const;

    // MachGuiStartupScreens should query this to find out if a key has been
    // pressed that causes the ingame menus to be displayed.
    bool switchToMenus() const;
    void resetSwitchToMenus();
    // Cause menus to be displayed.
    void switchToMenus(bool);

    // Don't bother going through all the menu screens to exit
    void instantExit(bool);
    bool instantExit() const;

    // Go into machines head and control in 1st person.
    void switchToInHead();

    void checkSwitchGuiRoot();

    // When a game is in progress and we have been in the menu screens this function
    // is called to return to the in game screen.
    void switchBackToInGame();

    // True if the inGame gui is currently in first person context.
    bool inFirstPerson() const;

    // Position of mouse cursor when right button was pressed.
    const Gui::Coord& rightClickMousePos() const;

    // Called by the first person code because the ingame screen still needs to do a certain amount
    // of updates. e.g. the map must continue to unfog the fog-of-war. Because the first person main object
    // is also a GuiRoot it gets the GuiManager::update calls and the MachInGameScreen stop getting update
    // calls, hence the need for this function.
    void updateWhilstInFirstPerson();

    // Get at control panel.
    MachGuiControlPanel& controlPanel();
    // PRE( pControlPanel_ );

    // Switch control panel on and off
    bool controlPanelOn() const;
    void controlPanelOn(bool);

    // Whether the interface is drawn over the world. With it off the world view
    // grows to the whole window and nothing goes on top of it: no panels, no
    // prompt, no text, not even the pointer. For photographing the world at the
    // full size of the window.
    //
    // The console is the exception, since it is what the world is framed from
    // while the rest is gone. It is not in the frame a screenshot is taken from,
    // so it can be left open to take one.
    //
    // Only the drawing stops. What cannot be seen still takes clicks, so drive
    // the camera from the console rather than the mouse while it is off.
    void setUiVisible(bool visible);
    bool isUiVisible() const;

    // Call when the ingame resolution has been changed
    void resolutionChange();

    // Get an overview of what is in the corral
    enum CorralState
    {
        CORRAL_EMPTY = 0x0000,
        CORRAL_ONEENTRY = 0x0001,
        CORRAL_ALLMACHINES = 0x0002,
        CORRAL_ALLCONSTRUCTIONS = 0x0004,
        CORRAL_ALLFACTORIES = 0x0008,
        CORRAL_ALLCONSTRUCTORS = 0x0010,
        CORRAL_ALLAPCS = 0x0020,
        CORRAL_SOMEMACHINES = 0x0040,
        CORRAL_SOMECONSTRUCTIONS = 0x0080,
        CORRAL_SOMEENEMY = 0x0100,
        CORRAL_SOMEFRIENDLY = 0x0200
    };

    using CorralStateBitfield = int;
    CorralStateBitfield corralState() const;

    // Called to initiate a screen shot.  If hight-quality rendering options
    // are supported but not in use (e.g. antialiasing), this sets said options,
    // sets isRenderingScreenShot, then just continues.  In the absence of
    // fancy rendering options this writes the shot immediately.
    // The shot is named as asked, or numbered after the ones already taken when
    // no name is given.
    void initiateScreenShot(std::string fileName = {});
    bool isRenderingScreenShot() const;

    // If isRenderingScreenShot is true, then this should be called to
    // do the actual saving of the shot.
    // PRE(isRenderingScreenShot()); POST(!isRenderingScreenShot());
    void finalizeScreenShot();

    Gui::Box getWorldViewWindowVisibleArea() const;

    // Used to determine if "Network Busy" message should be displayed
    bool isNetworkStuffed() const;

    void disableFirstPerson(bool);
    bool isFirstPersonDisabled() const;

protected:
    // Update whether the game has been lost or won
    void updateGameState();

    // inherited from GuiDisplayable...
    void doDisplay() override;

    // inherited from GuiRoot...
    void doBecomeRoot() override;
    void doBecomeNotRoot() override;

    // Respond to any key presses. Pass key events on to Motion classes.
    bool doHandleKeyEvent(const GuiKeyEvent& e) override;
    // Any key presses considered none permenant
    bool doHandleKeyEventHacks(const GuiKeyEvent& e);

    void doHandleContainsMouseEvent(const GuiMouseEvent&) override;

    void updateChildVisible();

    // True if the root is handling right mouse clicks, and they should not
    // be dispatched according to location
    bool doHandleRightClickEvent(const GuiMouseEvent&) override;

    // Inherited from W4dObserver
    // true iff this observer is to exist in this subject's list of observers
    // following this call. This will typically be implemented using double dispatch.
    // The clientData is of interest only if event == CLIENT_SPECIFIC.Interpretation
    // is client defined.
    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    // Informs observer that an observed domain is being deleted.
    // This observer need not call the W4dDomain::detach() method - this
    // will be done automatically.
    void domainDeleted(W4dDomain* pDomain) override;

    // Helper methods for actor prompt text
    bool addPromptTextMachineInfo(const MachActor* pActor, std::string& prompt);
    bool addPromptTextConstructionInfo(const MachActor* pActor, std::string& prompt);
    bool addPromptTextArtefactInfo(const MachActor* pActor, std::string& prompt);
    bool addPromptTextDebrisInfo(const MachActor* pActor, std::string& prompt);
    bool addPromptTextOreHolographInfo(const MachActor* pActor, std::string& prompt);

    // Should control panel be displayed ( certain modes require the control panel to be
    // revealed, i.e. construction menu ).
    bool displayControlPanel() const;

    void toggleConsoleDropDown();

private:
    // When an actor is selected, this should be called to ensure that
    // the navigator is dismissed at the correct time
    void checkDismissNavigator();

    // Initialise the allCommands collection
    void initialiseAllCommands();

    // Do some of the deselection work
    void unselect(MachActor* pActor);

    // Add/remove display of various banks where appropriate.
    void setupActorBank();

    // Create/Update camera scroll regions of screen
    void setupCameraScrollAreas();

    // Create/Update prompt text ( called after resolution change )
    void setupPromptText();
    void updateConsoleDropDownViewport();

    void setupChatMessages();

    void setupNavigators();

    void setupCorralAndCommandIcons();

    void updateCorralState();

    // Physically save a screen shot, cf. initiateScreenShot and finalizeScreenShot.
    void saveScreenShot();

    // Data members...
    W4dSceneManager* pSceneManager_{}; // The 3d window scene maanger
    W4dRoot* pRoot_{};
    System::IConsole* console_{};
    MachWorldViewWindow* pWorldViewWindow_{};
    MachGuiBmuButton* pBmuButton_{};
    Actors selectedActors_; // The collection of currently selected actors
    Commands allCommands_; // The collection of all the potential commands
    std::unique_ptr<MachGuiCommand> pDefaultCommand_; // The command that is active when no other command is active
    std::unique_ptr<MachGuiCommand> pActiveCommand_; // The active command. Null => use pDefaultCommand_.
    W4dDomain::EntityFilter cursorFilter_{}; // The filter to be used for findNearerEntity
    bool commandBankNeedsUpdating_{}; // True when the visibility of command icons needs to be updated
    bool cancelActiveCommand_{};
    bool applyCommandToSquadron_{};
    bool switchToMenus_{}; // Indicates to MachGuiStartupScreens that they should become root
    bool switchGuiRoot_{};
    bool inFirstPerson_{};
    MachActor* pHighlightedActor_{};
    MachContinentMap* pContinentMap_{};
    MachMachinesIcon* pMachinesIcon_{};
    MachConstructionsIcon* pConstructionsIcon_{};
    MachSquadronIcon* pSquadronIcon_{};
    MachPromptText* pPromptText_{}; // The displayable displaying the prompt text
    MachGuiCorralSingleIcon* pCorralSingleIcon_{};
    MachGuiCorral* pCorral_{};
    MachGuiMachineNavigator* pMachineNavigation_{};
    MachGuiConstructionNavigator* pConstructionNavigation_{};
    MachCommandIcons* pCommandIcons_{};
    MachSmallCommandIcons* pSmallCommandIcons_{};
    MachConstructMenu* pConstructMenu_{};
    MachBuildMenu* pBuildMenu_{};
    MachHWResearchMenu* pHWResearchMenu_{};
    MachProductionBank* pProductionBank_{}; // A factory production bank
    MachHWResearchBank* pHWResearchBank_{}; // A hardware lab research bank
    MachGuiSquadronBank* pSquadronBank_{}; // The squadron bank
    std::unique_ptr<MachCameras> pCameras_;
    MachGui::ControlPanelContext controlPanelContext_{}; // Menu context
    std::unique_ptr<MachInGameCursors2d> pCursors2d_; // The class enabling switching of cursor
    GameState gameState_{};
    PhysAbsoluteTime gameStateTimer_{};
    MachGuiDefconCommand* pDefconCommand_{};
    MachGuiSelfDestructCommand* pSelfDestructCommand_{};
    MachGuiIonAttackCommand* pIonAttackCommand_{};
    MachGuiNukeAttackCommand* pNukeAttackCommand_{};
    std::unique_ptr<MachGuiFirstPerson> pFirstPerson_;
    Gui::Coord rightClickMousePos_{}; // When right click happens the coord is stored in this var.
    const MachActor* pPromptTextActor_{}; // Actor whos info is on the prompt text
    MachGuiControlPanelAddOn* pControlPanelAddOn_{};
    bool controlPanelOn_{};
    bool uiVisible_{};
    int controlPanelXPos_{};
    MachGuiControlPanel* pControlPanel_{};
    MachGuiConsoleDropDown* pConsoleDropDown_{};
    int consoleDropDownOffset_{};
    MachGuiMapArea* pMapArea_{};
    MachGuiCameraScrollArea* pTopCameraScrollArea_{};
    MachGuiCameraScrollArea* pBottomCameraScrollArea_{};
    MachGuiCameraScrollArea* pLeftCameraScrollArea_{};
    MachGuiCameraScrollArea* pRightCameraScrollArea_{};
    bool resolutionChanged_{};
    bool renderingScreenShot_{};
    bool screenShotToggledAA_{};
    std::string screenShotName_{};
    CorralStateBitfield corralState_{};
    MachGuiInGameChatMessagesDisplay* pChatMessageDisplay_{};
    int redrawMapCounter_{}; // When ingame becomes GuiRoot the map needs to be redrawn every frame for 1st 4 frames to
                             // remove graphic glich (hack!! Oh well.)
    double networkStuffedStartTime_{};
    bool disableFirstPerson_{};
    bool instantExit_{};
    Utils::CallbackHandleUPtr renderStatsHandle_{};

#ifndef PRODUCTION
    bool showCurrentMachine_{};
    bool showNetworkStuffed_{};
#endif
};

#endif

/* End INGAME.HPP ***************************************************/
