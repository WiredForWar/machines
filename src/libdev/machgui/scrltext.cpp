/*
 * S C R L T E X T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/scrltext.hpp"

#include "base/diag.hpp"
#include "machgui/startup.hpp"
#include "machgui/menutext.hpp"
#include "machgui/text.hpp"
#include "gui/restring.hpp"
#include "gui/font.hpp"
#include "gui/painter.hpp"
#include "machgui/menus_helper.hpp"

MachGuiScrollableText::MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, uint stringId)
    : GuiSimpleScrollableList(pParent, box, 1000, GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1, 1)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    setText(stringId);

    TEST_INVARIANT;
}

MachGuiScrollableText::MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, const string& text)
    : GuiSimpleScrollableList(pParent, box, 1000, GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1, 1)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    setText(text);

    TEST_INVARIANT;
}

MachGuiScrollableText::MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box)
    : GuiSimpleScrollableList(pParent, box, 1000, GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1, 1)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

MachGuiScrollableText::MachGuiScrollableText(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    uint columnWidth,
    const string& text)
    : GuiSimpleScrollableList(
        pParent,
        box,
        columnWidth,
        GuiBmpFont::getFont("gui/menu/smallfnt.bmp").charHeight() + 1,
        1)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    setText(text);

    TEST_INVARIANT;
}

void MachGuiScrollableText::setText(uint stringId)
{
    GuiResourceString text(stringId);
    setText(text.asString());
}

void MachGuiScrollableText::setText(const string& text)
{
    deleteAllChildren();

    strings linesOfText;
    linesOfText.reserve(64);
    MachGuiMenuText::chopUpText(text, width(), GuiBmpFont::getFont("gui/menu/smallfnt.bmp"), &linesOfText);

    for (auto iter = linesOfText.begin(); iter != linesOfText.end(); ++iter)
    {
        string lineOfText = *iter;

        NEIL_STREAM(lineOfText << std::endl);

        if (strncasecmp(&lineOfText.c_str()[0], "<w>", 3) == 0)
        {
            new MachGuiText(this, width(), &lineOfText.c_str()[3], "gui/menu/smalwfnt.bmp");
        }
        else
        {
            new MachGuiText(this, width(), lineOfText);
        }
    }

    childrenUpdated();
}

MachGuiScrollableText::~MachGuiScrollableText()
{
    TEST_INVARIANT;
}

void MachGuiScrollableText::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiScrollableText& t)
{

    o << "MachGuiScrollableText " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiScrollableText " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachGuiScrollableText::doDisplay()
{
    // Blit background to list box item
    auto* shared = pRootParent_->getSharedBitmaps();
    auto backdrop = shared->getNamedBitmap("backdrop");
    shared->blitNamedBitmapFromArea(
        backdrop,
        absoluteBoundary(),
        absoluteBoundary().minCorner(),
        [shared, backdrop](const Gui::Box& box) {
            using namespace machgui::helper::menus;
            return centered_bitmap_transform(
                box,
                shared->getWidthOfNamedBitmap(backdrop),
                shared->getHeightOfNamedBitmap(backdrop));
        });
}

/* End SCRLTEXT.CPP *************************************************/
