#pragma once

#include "gui/Displayable.hpp"
#include "machlog/Messaging/FirstPersonHandler.hpp"

class MachGuiFPCommand : public GuiDisplayable
{
public:
    enum class CommandIconState : int
    {
        INVALID,
        VALID,
        ACTIVATED,
        NUM_STATES
    };
    static constexpr int NumCommandIconStates { static_cast<int>(CommandIconState::NUM_STATES) };

    ///////////////////////////////////////////////////

    MachGuiFPCommand(GuiDisplayable* pParent, const Gui::Coord& relPos);
    ~MachGuiFPCommand() override;
    // NON-COPYABLE
    MachGuiFPCommand(const MachGuiFPCommand&) = delete;
    MachGuiFPCommand& operator=(const MachGuiFPCommand&) = delete;

    ///////////////////////////////////////////////////

    void logHandler(MachLog1stPersonHandler* logHandler);
    void resetLogHandler();

    // Change the icon that displays within the control
    void updateSquadIcon();
    void clearSquadIcon();

    // Update the displayed number that shows which squad is selected
    void updateSquadNumber();
    void resetSquadNumber();

    // Change command icon states
    void setAttackIconState(CommandIconState state);
    void setFollowIconState(CommandIconState state);
    void setMoveIconState(CommandIconState state);

protected:
    void doDisplay() override;

private:
    static GuiBitmap& noSquadronSelected();
    static GuiBitmap& widgetBody();

    static GuiBitmap* attackCommandIcons();
    static GuiBitmap* followCommandIcons();
    static GuiBitmap* moveCommandIcons();

    CommandIconState attackCommandState_;
    CommandIconState followCommandState_;
    CommandIconState moveCommandState_;

    GuiBitmap activeSquadIcon_;

    // Will either point to noSquadronSelected() OR activeSquadIcon_
    GuiBitmap* pActiveSquadIcon_;

    // Squadron ids run 1...10 and stand for the digits 0...9; 0 means no squad and draws nothing.
    int64_t activeSquadNumber_{};
    MachLog1stPersonHandler* pLogHandler_ {};
};
