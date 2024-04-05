/*
 * C O M M B A N K . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "stdlib/string.hpp"
#include "mathex/point3d.hpp"
#include "machgui/commbank.hpp"
#include "machgui/command.hpp"
#include "machgui/gui.hpp"
#include "machgui/guictrl.hpp"
#include "machgui/ingame.hpp"
#include "machgui/cmddefcn.hpp"
#include "machgui/cmddestr.hpp"
#include "machgui/cmdionat.hpp"
#include "machgui/cmdnukat.hpp"
#include "machgui/controlp.hpp"
#include "gui/restring.hpp"
#include "gui/painter.hpp"
#include "machgui/internal/mgsndman.hpp"

static constexpr int MaxCommandsNumber = 14;
static constexpr int CommandsPerRow = 2;
static constexpr int CommandsMaxRows = MaxCommandsNumber / CommandsPerRow;

///////////////////////////////////////////////////////////////////////////////////////

class MachGuiNukeAttackCommandIcon : public MachCommandIcon
{
public:
    MachGuiNukeAttackCommandIcon(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const MachGuiCommand& command,
        MachInGameScreen* pInGameScreen)
        : MachCommandIcon(pParent, rel, command, pInGameScreen)
    {
    }

    void doDisplayInteriorEnabled(const Gui::Coord& abs) override
    {
        MachCommandIcon::doDisplayInteriorEnabled(abs);

        static GuiBitmap rechargeBmp = MachGui::getScaledImage("gui/commands/nukeatkr.bmp");

        float displayWidth
            = ((float)rechargeBmp.width() / 100.0) * (float)MachGuiNukeAttackCommand::highestPercentageRecharged();

        GuiPainter::instance().blit(rechargeBmp, Gui::Box(0, 0, displayWidth, rechargeBmp.height()), abs);
        GuiPainter::instance()
            .verticalLine(Gui::Coord(abs.x() + displayWidth, abs.y()), rechargeBmp.height(), Gui::BLACK(), 1);
    }
};

///////////////////////////////////////////////////////////////////////////////////////

class MachGuiIonAttackCommandIcon : public MachCommandIcon
{
public:
    MachGuiIonAttackCommandIcon(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const MachGuiCommand& command,
        MachInGameScreen* pInGameScreen)
        : MachCommandIcon(pParent, rel, command, pInGameScreen)
    {
    }

    void doDisplayInteriorEnabled(const Gui::Coord& abs) override
    {
        MachCommandIcon::doDisplayInteriorEnabled(abs);

        static GuiBitmap rechargeBmp = MachGui::getScaledImage("gui/commands/ionattkr.bmp");

        float displayWidth
            = ((float)rechargeBmp.width() / 100.0) * (float)MachGuiIonAttackCommand::highestPercentageRecharged();

        GuiPainter::instance().blit(rechargeBmp, Gui::Box(0, 0, displayWidth, rechargeBmp.height()), abs);
        GuiPainter::instance()
            .verticalLine(Gui::Coord(abs.x() + displayWidth, abs.y()), rechargeBmp.height(), Gui::BLACK(), 1);
    }
};

///////////////////////////////////////////////////////////////////////////////////////

class MachGuiDefconCommandIcon : public MachCommandIcon
{
public:
    MachGuiDefconCommandIcon(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const MachGuiCommand& command,
        MachInGameScreen* pInGameScreen)
        : MachCommandIcon(pParent, rel, command, pInGameScreen)
    {
    }

    /////////////////////////////////////////////////
    // Inherited from GuiBitmapButtonWithFilledBorder
    const GuiBitmap& getBitmap() const override
    {
        static GuiBitmap bmpDefcon1 = MachGui::getScaledImage("gui/commands/defco1.bmp");
        static GuiBitmap bmpDefcon2 = MachGui::getScaledImage("gui/commands/defco2.bmp");
        static GuiBitmap bmpDefcon3 = MachGui::getScaledImage("gui/commands/defco3.bmp");
        static GuiBitmap bmpDefcon12 = MachGui::getScaledImage("gui/commands/defco12.bmp");
        static GuiBitmap bmpDefcon23 = MachGui::getScaledImage("gui/commands/defco23.bmp");
        static GuiBitmap bmpDefcon123 = MachGui::getScaledImage("gui/commands/defco123.bmp");
        static GuiBitmap bmpDefcon13 = MachGui::getScaledImage("gui/commands/defco13.bmp");

        if (MachGuiDefconCommand::defconLow() && MachGuiDefconCommand::defconNormal()
            && MachGuiDefconCommand::defconHigh())
            return bmpDefcon123;
        else if (MachGuiDefconCommand::defconLow() && MachGuiDefconCommand::defconNormal())
            return bmpDefcon12;
        else if (MachGuiDefconCommand::defconNormal() && MachGuiDefconCommand::defconHigh())
            return bmpDefcon23;
        else if (MachGuiDefconCommand::defconLow() && MachGuiDefconCommand::defconHigh())
            return bmpDefcon13;
        else if (MachGuiDefconCommand::defconLow())
            return bmpDefcon1;
        else if (MachGuiDefconCommand::defconNormal())
            return bmpDefcon2;

        return bmpDefcon3;
    }

    void doHandleContainsMouseEvent(const GuiMouseEvent&) override
    {
        // Load the string for the command
        GuiResourceString::Id id = pCommand()->cursorPromptStringId();
        GuiResourceString prompt(id);

        // Set the cursor prompt
        pCommand()->inGameScreen().cursorPromptText(prompt.asString());
    }
};

///////////////////////////////////////////////////////////////////////////////////////

MachCommandIcons::MachCommandIcons(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
    : GuiScrollableIconSequence(
        pParent,
        translateBox(
            arrayDimensions(
                MachCommandIcon::reqWidth() * 2 + MachCommandIcons::horizontalSpacing(),
                MachCommandIcon::reqHeight(),
                CommandsMaxRows,
                1),
            rel),
        staticCoords(),
        1)
    , pInGameScreen_(pInGameScreen)
{
    // Construct an icon for every command available in the inGameScreen
    const MachInGameScreen::Commands& commands = pInGameScreen->allCommands();

    for (std::size_t i = 0; i != commands.size(); ++i)
    {
        if (commands[i] == pInGameScreen->defconCommand())
        {
            new MachGuiDefconCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->selfDestructCommand())
        {
            new MachGuiSelfDestructCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->ionAttackCommand())
        {
            new MachGuiIonAttackCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->nukeAttackCommand())
        {
            new MachGuiNukeAttackCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else
        {
            new MachCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
    }
}

MachCommandIcons::~MachCommandIcons()
{
    // Intentionally Empty
}

const GuiIconSequence::Coords& MachCommandIcons::coords() const
{
    return staticCoords();
}

// static
const GuiIconSequence::Coords& MachCommandIcons::staticCoords()
{
    static Coords coords_ = arrayCoords(
        MachCommandIcon::reqWidth() + MachCommandIcons::horizontalSpacing(),
        MachCommandIcon::reqHeight() + 1,
        CommandsMaxRows,
        CommandsPerRow);
    return coords_;
}

void MachCommandIcons::doDisplay()
{
    pInGameScreen_->controlPanel().redrawAreaImmediate(*this);
    GuiIconSequence::doDisplay();
}

void MachCommandIcons::change()
{
    changed();
}

// static
size_t MachCommandIcons::reqWidth()
{
    return (2 * MachCommandIcon::reqWidth()) + horizontalSpacing();
}

// static
size_t MachCommandIcons::reqHeight()
{
    return (9 * MachCommandIcon::reqHeight());
}

size_t MachCommandIcons::horizontalSpacing()
{
    return 5 * MachGui::uiScaleFactor();
}

/* //////////////////////////////////////////////////////////////// */

MachSmallCommandIcons::MachSmallCommandIcons(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    MachInGameScreen* pInGameScreen)
    : GuiScrollableIconSequence(
        pParent,
        translateBox(arrayDimensions(MachCommandIcons::reqWidth(), MachCommandIcon::reqHeight(), 1, 1), rel),
        staticCoords(),
        1)
    , pInGameScreen_(pInGameScreen)
{
    // Construct an icon for every command available in the inGameScreen
    int i = 0;
    const MachInGameScreen::Commands& commands = pInGameScreen->allCommands();

    for (std::size_t i = 0; i != commands.size(); ++i)
    {
        if (commands[i] == pInGameScreen->defconCommand())
        {
            new MachGuiDefconCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->selfDestructCommand())
        {
            new MachGuiSelfDestructCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->ionAttackCommand())
        {
            new MachGuiIonAttackCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else if (commands[i] == pInGameScreen->nukeAttackCommand())
        {
            new MachGuiNukeAttackCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
        else
        {
            new MachCommandIcon(this, Gui::Coord(0, 0), *commands[i], pInGameScreen);
        }
    }
}

MachSmallCommandIcons::~MachSmallCommandIcons()
{
    // Intentionally Empty
}

const GuiIconSequence::Coords& MachSmallCommandIcons::coords() const
{
    return staticCoords();
}

// static
const GuiIconSequence::Coords& MachSmallCommandIcons::staticCoords()
{
    static Coords coords_ = arrayCoords(
        MachCommandIcon::reqWidth() + MachCommandIcons::horizontalSpacing(),
        MachCommandIcon::reqHeight() + 1,
        CommandsMaxRows,
        CommandsPerRow);
    return coords_;
}

void MachSmallCommandIcons::doDisplay()
{
    pInGameScreen_->controlPanel().redrawAreaImmediate(*this);
    GuiIconSequence::doDisplay();
}

void MachSmallCommandIcons::change()
{
    changed();
}

// static
size_t MachSmallCommandIcons::reqWidth()
{
    return (2 * MachCommandIcon::reqWidth()) + MachCommandIcons::horizontalSpacing();
}

// static
size_t MachSmallCommandIcons::reqHeight()
{
    return MachCommandIcon::reqHeight();
}

////////////////////////////////////////////////////////////////////

MachCommandIcon::MachCommandIcon(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    const MachGuiCommand& command,
    MachInGameScreen* pInGameScreen)
    : GuiBitmapButtonWithFilledBorder(
        pParent,
        rel,
        GuiBorderMetrics(1, 1, 1),
        GuiFilledBorderColours(Gui::BLACK(), Gui::DARKGREY(), Gui::DARKGREY(), Gui::RED(), Gui::BLACK()),
        MachGui::getScaledImage(bitmapPaths(command).first),
        Gui::Coord(1, 1))
    , pInGameScreen_(pInGameScreen)
    , pCommand_(&command)
{
    // Intentionally Empty
}

MachCommandIcon::~MachCommandIcon()
{
    // Intentionally Empty
}

MachInGameScreen& MachCommandIcon::inGameScreen()
{
    return *pInGameScreen_;
}

const MachInGameScreen& MachCommandIcon::inGameScreen() const
{
    return *pInGameScreen_;
}

std::pair<std::string, std::string> MachCommandIcon::bitmapPaths(const MachGuiCommand& command) const
{
    const std::pair<string, string> strings = command.iconNames();
    return { strings.first, strings.first };
}

// virtual
void MachCommandIcon::doBeDepressed(const GuiMouseEvent&)
{
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

// virtual
void MachCommandIcon::doBeReleased(const GuiMouseEvent&)
{
    // Make the command active
    MachInGameScreen& inGameScreen = pCommand_->inGameScreen();
    inGameScreen.activeCommand(*pCommand_);
}

// virtual
bool MachCommandIcon::isEligableForVisibility() const
{
    return (pCommand_->isVisible() && GuiBitmapButtonWithFilledBorder::isEligableForVisibility());
}

// virtual
void MachCommandIcon::doHandleMouseEnterEvent(const GuiMouseEvent&)
{
    // Load the string for the command
    GuiResourceString::Id id = pCommand_->cursorPromptStringId();
    GuiResourceString prompt(id);

    // Set the cursor prompt
    pCommand_->inGameScreen().cursorPromptText(prompt.asString());
}

// virtual
void MachCommandIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pCommand_->inGameScreen().clearCursorPromptText();

    GuiBitmapButtonWithFilledBorder::doHandleMouseExitEvent(mouseEvent);
}

size_t MachCommandIcon::reqWidth()
{
    return 77 * MachGui::uiScaleFactor(); // TODO : remove hardcoding
}

// static
size_t MachCommandIcon::reqHeight()
{
    return 13 * MachGui::uiScaleFactor(); // TODO : remove hardcoding
}

const MachGuiCommand* MachCommandIcon::pCommand() const
{
    return pCommand_;
}

/* //////////////////////////////////////////////////////////////// */

/* End COMMBANK.CPP *************************************************/
