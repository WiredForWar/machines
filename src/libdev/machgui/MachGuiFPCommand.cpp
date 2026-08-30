#include "machgui/MachGuiFPCommand.hpp"

#include "system/PathName.hpp"
#include "gui/GuiPainter.hpp"
#include "machlog/Actors/Machine.hpp"
#include "machgui/gui.hpp"
#include "machgui/ui/MenuStyle.hpp"
#include "world4d/Manager.hpp"
#include "world4d/Scene/SceneManager.hpp"
#include "render/Device.hpp"
#include "render/TextOptions.hpp"
#include "machlog/Messaging/MachLog1stPersonActiveSquad.hpp"

#include <string>

MachGuiFPCommand::MachGuiFPCommand(GuiDisplayable* pParent, const Gui::Coord& relPos)
    : GuiDisplayable(pParent, Gui::Box(relPos, 1, 1))
{
    pActiveSquadIcon_ = &noSquadronSelected();

    attackCommandState_ = CommandIconState::INVALID;
    followCommandState_ = CommandIconState::INVALID;
    moveCommandState_ = CommandIconState::INVALID;
}

MachGuiFPCommand::~MachGuiFPCommand()
{
}

void MachGuiFPCommand::logHandler(MachLog1stPersonHandler* logHandler)
{
    pLogHandler_ = logHandler;
}

void MachGuiFPCommand::resetLogHandler()
{
    pLogHandler_ = nullptr;
}

void MachGuiFPCommand::updateSquadIcon()
{
    if (!pLogHandler_)
    {
        return;
    }

    MachLogMachine* squadLeader = pLogHandler_->getActiveSquadron().getLeadingMachine();

    if (squadLeader)
    {
        activeSquadIcon_ = MachGui::createIconForMachine(squadLeader, true);
        pActiveSquadIcon_ = &activeSquadIcon_;
        return;
    }

    // No squad leader? no squad!
    pActiveSquadIcon_ = &noSquadronSelected();
}

void MachGuiFPCommand::clearSquadIcon()
{
    pActiveSquadIcon_ = &noSquadronSelected();
}

void MachGuiFPCommand::updateSquadNumber()
{
    if (!pLogHandler_)
    {
        return;
    }

    activeSquadNumber_ = pLogHandler_->getActiveSquadron().getActiveSquadronId();
}

void MachGuiFPCommand::resetSquadNumber()
{
    activeSquadNumber_ = 0;
}

void MachGuiFPCommand::setAttackIconState(CommandIconState state)
{
    attackCommandState_ = state;
}
void MachGuiFPCommand::setFollowIconState(CommandIconState state)
{
    followCommandState_ = state;
}
void MachGuiFPCommand::setMoveIconState(CommandIconState state)
{
    moveCommandState_ = state;
}

// virtual
void MachGuiFPCommand::doDisplay()
{
    // Display widget body, active squad icon, etc...
    const auto topLeft = absoluteBoundary().topLeft();

    GuiPainter::instance().blit(widgetBody(), topLeft);
    GuiPainter::instance().blit(*pActiveSquadIcon_, topLeft + Gui::Coord(43, 50) * Gui::uiScaleFactor());

    // Squadron ids run 1 to 10 and stand for the digits 0 to 9; zero means no squad.
    if (activeSquadNumber_ > 0)
    {
        GuiPainter::instance().drawText(
            topLeft + Gui::Coord(64, 20) * Gui::uiScaleFactor(),
            std::to_string(activeSquadNumber_ - 1),
            Gui::TextOptions(MachGui::Menu::focusedTextColor()),
            MachGui::Menu::font());
    }

    // widget.bmp: 130x130
    // the command icons: 64x24
    int state = static_cast<int>(moveCommandState_);
    GuiPainter::instance().blit(moveCommandIcons()[state], topLeft + Gui::Coord(0, 130) * Gui::uiScaleFactor());
    state = static_cast<int>(followCommandState_);
    GuiPainter::instance().blit(followCommandIcons()[state], topLeft + Gui::Coord(66, 130) * Gui::uiScaleFactor());
    state = static_cast<int>(attackCommandState_);
    GuiPainter::instance().blit(
        attackCommandIcons()[state],
        topLeft + Gui::Coord(130 / 2 - 64 / 2, 130 + 24) * Gui::uiScaleFactor());
}

// static
GuiBitmap& MachGuiFPCommand::noSquadronSelected()
{
    static GuiBitmap blankIcon = Gui::getScaledImage("gui/fstpersn/machines/backgrnd.bmp");
    return blankIcon;
}

// static
GuiBitmap& MachGuiFPCommand::widgetBody()
{
    static GuiBitmap widget = Gui::getScaledImage("gui/fstpersn/command/widget.bmp");
    return widget;
}

// static
GuiBitmap* MachGuiFPCommand::attackCommandIcons()
{
    static GuiBitmap attackIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/attack_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/attack_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/attack_activate.bmp"),
    };

    return attackIcons;
}

// static
GuiBitmap* MachGuiFPCommand::followCommandIcons()
{
    static GuiBitmap followIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/follow_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/follow_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/follow_activate.bmp"),
    };

    return followIcons;
}

// static
GuiBitmap* MachGuiFPCommand::moveCommandIcons()
{
    static GuiBitmap moveIcons[NumCommandIconStates] = {
        /* INVALID   */ Gui::getScaledImage("gui/fstpersn/command/move_invalid.bmp"),
        /* VALID     */ Gui::getScaledImage("gui/fstpersn/command/move_valid.bmp"),
        /* ACTIVATED */ Gui::getScaledImage("gui/fstpersn/command/move_activate.bmp"),
    };

    return moveIcons;
}
