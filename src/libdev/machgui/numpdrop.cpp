/*
 * N U M P D R O P . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/numpdrop.hpp"
#include "machgui/ctxskirm.hpp"
#include "machgui/internal/strings.hpp"
#include "system/registry.hpp"
#include <cstring>

MachGuiNumPlayersDropDownListBoxCreator::MachGuiNumPlayersDropDownListBoxCreator(
    GuiDisplayable* pParent,
    MachGuiStartupScreens* pStartup,
    int width,
    const GuiStrings& availText)
    : MachGuiDropDownListBoxCreator(pParent, pStartup, width, availText)
{

    TEST_INVARIANT;
}

MachGuiNumPlayersDropDownListBoxCreator::MachGuiNumPlayersDropDownListBoxCreator(
    GuiDisplayable* pParent,
    MachGuiStartupScreens* pStartup,
    int width,
    const GuiStrings& availText,
    bool whiteFont)
    : MachGuiDropDownListBoxCreator(pParent, pStartup, width, availText, whiteFont)
{

    TEST_INVARIANT;
}

MachGuiNumPlayersDropDownListBoxCreator::MachGuiNumPlayersDropDownListBoxCreator(
    GuiDisplayable* pParent,
    MachGuiStartupScreens* pStartup,
    const Gui::Coord& relCoord,
    int width,
    const GuiStrings& availText,
    bool whiteFont,
    bool border)
    : MachGuiDropDownListBoxCreator(pParent, pStartup, relCoord, width, availText, whiteFont, border)
{

    TEST_INVARIANT;
}

MachGuiNumPlayersDropDownListBoxCreator::~MachGuiNumPlayersDropDownListBoxCreator()
{
    TEST_INVARIANT;
}

void MachGuiNumPlayersDropDownListBoxCreator::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
MachGuiDropDownList* MachGuiNumPlayersDropDownListBoxCreator::createDropDownList(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<std::string>& itemText,
    MachGuiDropDownListBoxCreator* listBoxCreator)
{
    return new MachGuiNumPlayersDropDownList(
        pParent,
        box,
        horizontalSpacing,
        verticalSpacing,
        scrollInc,
        itemWidth,
        itemText,
        listBoxCreator);
}

std::ostream& operator<<(std::ostream& o, const MachGuiNumPlayersDropDownListBoxCreator& t)
{

    o << "MachGuiNumPlayersDropDownListBoxCreator " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiNumPlayersDropDownListBoxCreator " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiNumPlayersDropDownListBoxCreator::doHandleNavigationKey(
    NavKey navKey,
    MachGuiFocusCapableControl** pFocusCapableControl)
{
    bool retValue = MachGuiDropDownListBoxCreator::doHandleNavigationKey(navKey, pFocusCapableControl);

    if (navKey == MachGuiFocusCapableControl::UP_ARROW || navKey == MachGuiFocusCapableControl::DOWN_ARROW)
    {
        GuiResourceString two(IDS_MENU_NUMPLAYERS2);
        GuiResourceString three(IDS_MENU_NUMPLAYERS3);
        GuiResourceString four(IDS_MENU_NUMPLAYERS4);

        if (strcasecmp(two.asString().c_str(), text().c_str()) == 0)
        {
            MachGuiCtxSkirmish::chosenNumPlayers() = 2;
        }
        else if (strcasecmp(three.asString().c_str(), text().c_str()) == 0)
        {
            MachGuiCtxSkirmish::chosenNumPlayers() = 3;
        }
        else if (strcasecmp(four.asString().c_str(), text().c_str()) == 0)
        {
            MachGuiCtxSkirmish::chosenNumPlayers() = 4;
        }
        else
        {
            ASSERT_INFO(text());
            ASSERT(false, "MachGuiDropDownListBoxCreator::doHandleNavigationKey has invalid text()");
        }

        SysRegistry::instance().setIntegerValue(
            "Game Settings\\Num Players",
            "Value",
            MachGuiCtxSkirmish::chosenNumPlayers());
    }

    return retValue;
}

MachGuiNumPlayersDropDownList::MachGuiNumPlayersDropDownList(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<std::string>& itemText,
    MachGuiDropDownListBoxCreator* listBoxCreator)
    : MachGuiDropDownList(
        pParent,
        box,
        horizontalSpacing,
        verticalSpacing,
        scrollInc,
        itemWidth,
        itemText,
        listBoxCreator)
{
    // Intentionally empty
}

MachGuiNumPlayersDropDownList::MachGuiNumPlayersDropDownList(
    MachGuiStartupScreens* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc,
    size_t itemWidth,
    const ctl_vector<std::string>& itemText,
    MachGuiDropDownListBoxCreator* listBoxCreator,
    bool whiteFont)
    : MachGuiDropDownList(
        pParent,
        box,
        horizontalSpacing,
        verticalSpacing,
        scrollInc,
        itemWidth,
        itemText,
        listBoxCreator,
        whiteFont)
{
    // Intentionally empty
}

// virtual
MachGuiDropDownListBoxItem* MachGuiNumPlayersDropDownList::createListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const std::string& text,
    bool whiteFont)
{
    return new MachGuiNumPlayersDropDownListBoxItem(pStartupScreens, pListBox, width, text, whiteFont);
}

MachGuiNumPlayersDropDownListBoxItem::MachGuiNumPlayersDropDownListBoxItem(
    MachGuiStartupScreens* pStartupScreens,
    MachGuiDropDownListBox* pListBox,
    size_t width,
    const std::string& text,
    bool whiteFont)
    : MachGuiDropDownListBoxItem(pStartupScreens, pListBox, width, text, whiteFont)
{
    // Intentionally empty
}

// virtual
void MachGuiNumPlayersDropDownListBoxItem::select()
{
    GuiResourceString two(IDS_MENU_NUMPLAYERS2);
    GuiResourceString three(IDS_MENU_NUMPLAYERS3);
    GuiResourceString four(IDS_MENU_NUMPLAYERS4);

    if (strcasecmp(two.asString().c_str(), text().c_str()) == 0)
    {
        MachGuiCtxSkirmish::chosenNumPlayers() = 2;
    }
    else if (strcasecmp(three.asString().c_str(), text().c_str()) == 0)
    {
        MachGuiCtxSkirmish::chosenNumPlayers() = 3;
    }
    else if (strcasecmp(four.asString().c_str(), text().c_str()) == 0)
    {
        MachGuiCtxSkirmish::chosenNumPlayers() = 4;
    }
    else
    {
        ASSERT_INFO(text());
        ASSERT(false, "MachGuiNumPlayersDropDownListBoxItem::select() has invalid text()");
    }

    SysRegistry::instance().setIntegerValue(
        "Game Settings\\Num Players",
        "Value",
        MachGuiCtxSkirmish::chosenNumPlayers());

    // This base class function indirecty deletes this object, therefore it is called last
    MachGuiDropDownListBoxItem::select();
}

/* End NUMPDROP.CPP ************************************************/
