/*
 * S C R L T E X T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ui/ScrollableText.hpp"

#include "base/diag.hpp"
#include "gui/font.hpp"
#include "gui/root.hpp"
#include "machgui/menus_helper.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/ui/Text.hpp"

static const std::string fontName = "gui/menu/smallfnt.bmp";

MachGuiScrollableText::MachGuiScrollableText(GuiDisplayable* pParent, const Gui::Box& box, uint columnWidth)
    : GuiSimpleScrollableList(pParent, box, columnWidth, GuiBmpFont::getFont(fontName).charHeight() + 1, 1)
{
    pRootParent_ = static_cast<GuiRoot*>(pParent->findRoot(this));
    TEST_INVARIANT;
}

void MachGuiScrollableText::setText(const ResolvedUiString& text)
{
    deleteAllChildren();

    strings linesOfText;
    linesOfText.reserve(64);
    MachGuiMenuText::chopUpText(text, width(), GuiBmpFont::getFont(fontName), &linesOfText);

    for (auto iter = linesOfText.begin(); iter != linesOfText.end(); ++iter)
    {
        const std::string &lineOfText = *iter;

        NEIL_STREAM(lineOfText << std::endl);

        if (strncasecmp(&lineOfText.c_str()[0], "<w>", 3) == 0)
        {
            new MachGuiText(this, width(), &lineOfText.c_str()[3], fontName);
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
