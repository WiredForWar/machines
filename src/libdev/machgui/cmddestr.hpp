/*
 * C M D D E S T R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachGuiSelfDestructCommand

    Implement interaction for the SelfDestruct command
*/

#ifndef _MACHGUI_CMDDESTR_HPP
#define _MACHGUI_CMDDESTR_HPP

#include "base/base.hpp"
#include "ctl/pvector.hpp"
#include "mathex/point3d.hpp"
#include "machgui/cmddefau.hpp"
#include "machgui/commbank.hpp"

// Forward refs
class MachActor;

class MachGuiSelfDestructCommandIcon : public MachCommandIcon
{
public:
    MachGuiSelfDestructCommandIcon(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const MachGuiCommand& command,
        MachInGameScreen* pInGameScreen);
    ~MachGuiSelfDestructCommandIcon() override;

    const GuiBitmap& getBitmap() const override;

    using MachGuiSelfDestructCommandIcons = ctl_pvector<MachGuiSelfDestructCommandIcon>;

    static MachGuiSelfDestructCommandIcons& icons();
    static void refreshAllIcons();
};

// orthodox canonical (revoked)
class MachGuiSelfDestructCommand : public MachGuiDefaultCommand
{
public:
    // ctor.
    MachGuiSelfDestructCommand(MachInGameScreen* pInGameScreen);

    // dtor
    ~MachGuiSelfDestructCommand() override;

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Return new instance of this command
    MachGuiCommand* clone() const override;

    // The resource file id of the prompt to be displayed when the cursor moves
    // over the command icon
    uint cursorPromptStringId() const override;

    // The resource file id of the prompt to be displayed when the command is clicked
    uint commandPromptStringid() const override;

    // reference to the up and down bitmap names for this command
    const std::pair<string, string>& iconNames() const override;

    // Set the type data associated with the command.
    void typeData(MachLog::ObjectType objectType, int subType, uint level) override;

    // True if this is a kind of command that actor can ever execute.
    bool canActorEverExecute(const MachActor& actor) const override;

    // True if the interaction for the command is complete
    bool isInteractionComplete() const override;

    // Hot key processing
    bool processButtonEvent(const DevButtonEvent&) override;

    /////////////////////////////////////////////////

    enum ButtonState
    {
        BTN_HIDDEN,
        BTN_SHOWN,
        BTN_ACTIVE
    };
    static ButtonState buttonState();
    static bool hadFinalPick();

    static void resetButtonState();

    using Actors = ctl_pvector<MachActor>;

    static void update(const Actors& actors);

    static void stopPressed();

    bool actorsCanExecute() const override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachGuiSelfDestructCommand& t);

private:
    // Operation deliberately revoked
    MachGuiSelfDestructCommand(const MachGuiSelfDestructCommand&);
    MachGuiSelfDestructCommand& operator=(const MachGuiSelfDestructCommand&);
    bool operator==(const MachGuiSelfDestructCommand&);

    friend class MachGuiSelfDestructCommandIcon;

    static ButtonState& privateButtonState();
    static bool& privateHadFinalPick();
    static int& privateRotateFrame();
    static double& privateRotateFrameTime();

    /////////////////////////////////////////////////
    // Inherited from MachGuiCommand

    // Execute the command for pActor
    bool doApply(MachActor* pActor, string* pReason) override;
};

#endif

/* End CMDATTAC.HPP **************************************************/
