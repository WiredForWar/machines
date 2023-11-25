/*
 * G U I C T R L . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "gui/manager.hpp"

#include "gui/painter.hpp"
#include "gui/event.hpp"
#include "gui/restring.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/gui.hpp"
#include "machgui/guictrl.hpp"
#include "machgui/ingame.hpp"
#include "system/pathname.hpp"
#include "machlog/races.hpp"
#include "machlog/cntrl_pc.hpp"
#include "machlog/squad.hpp"
#include "ctl/pvector.hpp"
#include "machgui/internal/mgsndman.hpp"

MachGuiIconWithCounter::MachGuiIconWithCounter(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    const GuiBitmap& bitmap,
    MachInGameScreen* pInGameScreen)
    : GuiBitmapButtonWithFilledBorder(
        pParent,
        rel,
        GuiBorderMetrics(1, 1, 1),
        GuiFilledBorderColours(
            Gui::Colour(144.0 / 255.0, 148.0 / 255.0, 160.0 / 255.0),
            Gui::Colour(232.0 / 255.0, 232.0 / 255.0, 232.0 / 255.0),
            Gui::Colour(62.0 / 255.0, 62.0 / 255.0, 62.0 / 255.0),
            Gui::RED()),
        bitmap,
        Gui::Coord(1, 1))
    , pInGameScreen_(pInGameScreen)
{
    // Intentionally Empty
}

// virtual
MachGuiIconWithCounter::~MachGuiIconWithCounter()
{
    // Intentionally Empty
}

void MachGuiIconWithCounter::setCounterValue(size_t value)
{
    if (value == currentValue_)
        return;

    currentValue_ = value;
    changed();
}

// virtual
void MachGuiIconWithCounter::doDisplayInteriorEnabled(const Gui::Coord& abs)
{
    GuiBitmapButtonWithFilledBorder::doDisplayInteriorEnabled(abs);

    static GuiBitmap numbers[10] = {
        MachGui::getScaledImage("gui/navigate/numtop0.bmp"), MachGui::getScaledImage("gui/navigate/numtop1.bmp"),
        MachGui::getScaledImage("gui/navigate/numtop2.bmp"), MachGui::getScaledImage("gui/navigate/numtop3.bmp"),
        MachGui::getScaledImage("gui/navigate/numtop4.bmp"), MachGui::getScaledImage("gui/navigate/numtop5.bmp"),
        MachGui::getScaledImage("gui/navigate/numtop6.bmp"), MachGui::getScaledImage("gui/navigate/numtop7.bmp"),
        MachGui::getScaledImage("gui/navigate/numtop8.bmp"), MachGui::getScaledImage("gui/navigate/numtop9.bmp"),
    };

    Gui::Coord absCopy(abs);
    Gui::Vec offset(getBitmap().width(), getBitmap().width());
    absCopy += offset;

    MachGui::drawNumber(numbers, currentValue_, &absCopy);
}

MachMachinesIcon::MachMachinesIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
    : MachGuiIconWithCounter(pParent, rel, MachGui::getScaledImage("gui/navigate/red/machines.bmp"), pInGameScreen)
    , forcingUp_(false)
{
    popupButton(false);
}

MachMachinesIcon::~MachMachinesIcon()
{
    // Intentionally Empty
}

void MachMachinesIcon::doBeDepressed(const GuiMouseEvent&)
{
    pInGameScreen_->machineNavigationContext();
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

void MachMachinesIcon::doBeReleased(const GuiMouseEvent&)
{
    if (! forcingUp_)
    {
        pInGameScreen_->mainMenuOrSingleFactoryContext();
    }
}

void MachMachinesIcon::forceUp()
{
    if (isDepressed())
    {
        forcingUp_ = true;
        setDepressed(false);
        forcingUp_ = false;
    }
}

void MachMachinesIcon::refresh()
{
    // Get the values and update the dials
    MachLogRaces& races = MachLogRaces::instance();
    MachPhys::Race playerRace = races.pcController().race();

    size_t newValue = races.nMachines(playerRace);
    setCounterValue(newValue);
}

// virtual
void MachMachinesIcon::doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent)
{
    GuiBitmapButtonWithFilledBorder::doHandleMouseEnterEvent(mouseEvent);

    // Load the resource string
    GuiResourceString prompt(IDS_MACHINE_NAV);

    // Set the cursor prompt
    pInGameScreen_->cursorPromptText(prompt.asString());
}

// virtual
void MachMachinesIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pInGameScreen_->clearCursorPromptText();

    GuiBitmapButtonWithFilledBorder::doHandleMouseExitEvent(mouseEvent);
}

MachConstructionsIcon::MachConstructionsIcon(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    MachInGameScreen* pInGameScreen)
    : MachGuiIconWithCounter(pParent, rel, MachGui::getScaledImage("gui/navigate/red/construc.bmp"), pInGameScreen)
    , forcingUp_(false)
{
    popupButton(false);
}

MachConstructionsIcon::~MachConstructionsIcon()
{
    // Intentionally Empty
}

void MachConstructionsIcon::doBeDepressed(const GuiMouseEvent&)
{
    pInGameScreen_->constructionNavigationContext();
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

void MachConstructionsIcon::doBeReleased(const GuiMouseEvent&)
{
    if (! forcingUp_)
    {
        pInGameScreen_->mainMenuOrSingleFactoryContext();
    }
}

void MachConstructionsIcon::forceUp()
{
    if (isDepressed())
    {
        forcingUp_ = true;
        setDepressed(false);
        forcingUp_ = false;
    }
}

void MachConstructionsIcon::refresh()
{
    const MachLogRaces& races = MachLogRaces::instance();
    const MachPhys::Race playerRace = races.pcController().race();
    setCounterValue(races.nConstructions(playerRace));
}

// virtual
void MachConstructionsIcon::doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent)
{
    MachGuiIconWithCounter::doHandleMouseEnterEvent(mouseEvent);

    // Load the resource string
    GuiResourceString prompt(IDS_CONSTRUCTION_NAV);

    // Set the cursor prompt
    pInGameScreen_->cursorPromptText(prompt.asString());
}

// virtual
void MachConstructionsIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pInGameScreen_->clearCursorPromptText();

    MachGuiIconWithCounter::doHandleMouseExitEvent(mouseEvent);
}

/* //////////////////////////////////////////////////////////////// */

MachSquadronIcon::MachSquadronIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen)
    : MachGuiIconWithCounter(pParent, rel, MachGui::getScaledImage("gui/navigate/red/squads.bmp"), pInGameScreen)
    , pInGameScreen_(pInGameScreen)
{
    // Intentionally Empty
}

MachSquadronIcon::~MachSquadronIcon()
{
    // Intentionally Empty
}

void MachSquadronIcon::doBeDepressed(const GuiMouseEvent&)
{
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

void MachSquadronIcon::doBeReleased(const GuiMouseEvent&)
{
    // Clear all selections
    pInGameScreen_->deselectAll();

    // Enter the squadron menu
    pInGameScreen_->squadronContext();
}

void MachSquadronIcon::update()
{
    const MachLogRaces& races = MachLogRaces::instance();
    const MachPhys::Race race = races.pcController().race();

    size_t newValue = 0;
    for (MachLogSquadron* pSquadron : races.squadrons(race))
    {
        if (!pSquadron->machines().empty())
        {
            ++newValue;
        }
    }

    setCounterValue(newValue);
}

// virtual
void MachSquadronIcon::doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent)
{
    MachGuiIconWithCounter::doHandleMouseEnterEvent(mouseEvent);

    // Load the resource string
    GuiResourceString prompt(IDS_SQUAD_NAV);

    // Set the cursor prompt
    pInGameScreen_->cursorPromptText(prompt.asString());
}

// virtual
void MachSquadronIcon::doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent)
{
    // Clear the cursor prompt string
    pInGameScreen_->clearCursorPromptText();

    MachGuiIconWithCounter::doHandleMouseExitEvent(mouseEvent);
}

string MachGuiIconWithCounter::getRootDirectory() const
{
    string retValue = "gui/navigate/red/";

    if (MachLogRaces::instance().hasPCRace())
    {
        switch (MachLogRaces::instance().pcController().race())
        {
            case MachPhys::RED:
                break;
            case MachPhys::GREEN:
                retValue = "gui/navigate/green/";
                break;
            case MachPhys::BLUE:
                retValue = "gui/navigate/blue/";
                break;
            case MachPhys::YELLOW:
                retValue = "gui/navigate/yellow/";
                break;
                DEFAULT_ASSERT_BAD_CASE(MachLogRaces::instance().pcController().race());
        }
    }

    return retValue;
}

void MachConstructionsIcon::loadGame()
{
    bitmap(MachGui::getScaledImage(getRootDirectory() + "construc.bmp"));

    refresh();
}

void MachMachinesIcon::loadGame()
{
    bitmap(MachGui::getScaledImage(getRootDirectory() + "machines.bmp"));

    refresh();
}

void MachSquadronIcon::loadGame()
{
    bitmap(MachGui::getScaledImage(getRootDirectory() + "squads.bmp"));

    update();
}

/* End GUICTRL.CPP **************************************************/
