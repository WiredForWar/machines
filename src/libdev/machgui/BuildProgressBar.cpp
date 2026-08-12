/*
 * B U I L D B A R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/BuildProgressBar.hpp"
#include "machgui/gui.hpp"
#include "gui/Manager.hpp"
#include "gui/GuiPainter.hpp"

MachGuiBuildProgressBar::MachGuiBuildProgressBar(GuiDisplayable* pParent, const Gui::Coord& rel, size_t width)
    : GuiDisplayable(pParent, Gui::Box(rel, width, MachGuiBuildProgressBar::height()))
    , percentageComplete_(0.0)
{

    TEST_INVARIANT;
}

MachGuiBuildProgressBar::~MachGuiBuildProgressBar()
{
    TEST_INVARIANT;
}

void MachGuiBuildProgressBar::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void MachGuiBuildProgressBar::depress(bool doDepress)
{
    if (doDepress)
    {
        relativeCoord(Gui::Coord(3, 3));
    }
    else
    {
        relativeCoord(Gui::Coord(2, 2));
    }
}

Gui::Colour MachGuiBuildProgressBar::barColour() const
// width in pixels for a given ratio
{
    unsigned currentValue = 1;
    unsigned maxValue = 1;

    PRE(currentValue <= maxValue);
    const float LOW_THRESHOLD = 1.0 / 3.0;
    const float MID_THRESHOLD = 1.7 / 3.0;

    Gui::Colour result = MachGui::PROGRESSGREEN();
    float ratio = currentValue / (float)maxValue;
    if (ratio <= LOW_THRESHOLD)
        result = Gui::RED();
    else if (ratio <= MID_THRESHOLD)
        result = Gui::YELLOW();

    return result;
}

void MachGuiBuildProgressBar::doDisplay()
{
    const int borderThickness = MachGui::barBorderThickness();
    const int shadowThickness = MachGui::barShadowThickness();
    const int barThickness = MachGui::barValueLineThickness();

    GuiPainter::instance().filledRectangle(absoluteBoundary(), MachGui::PURPLE());

    const int interiorWidth = width() - (2 * borderThickness) - shadowThickness;
    const int barWidth = (percentageComplete_ / 100.0) * interiorWidth;

    // The shadow along the top and the bar under it, each drawn from its own top edge,
    // and the shadow down the side as deep as the two of them.
    const int left = absoluteCoord().x() + borderThickness + shadowThickness;
    const int top = absoluteCoord().y() + borderThickness;

    auto band = [](int x, int y, int bandWidth, int bandHeight, const Gui::Colour& colour) {
        if (bandWidth > 0)
            GuiPainter::instance().filledRectangle(Gui::Box(Gui::Coord(x, y), bandWidth, bandHeight), colour);
    };

    band(left, top, interiorWidth, shadowThickness, Gui::BLACK());
    band(left, top + shadowThickness, barWidth, barThickness, barColour());
    band(
        absoluteCoord().x() + borderThickness,
        top,
        shadowThickness,
        shadowThickness + barThickness,
        Gui::BLACK());

    GuiPainter::instance().hollowRectangle(absoluteBoundary(), Gui::WHITE(), borderThickness);
}

// static
size_t MachGuiBuildProgressBar::height()
{
    return MachGui::barBorderThickness() + MachGui::barShadowThickness() + MachGui::barValueLineThickness()
        + MachGui::barBorderThickness();
}

void MachGuiBuildProgressBar::percentageComplete(float complete)
{
    percentageComplete_ = complete;
    changed();
}

std::ostream& operator<<(std::ostream& o, const MachGuiBuildProgressBar& t)
{

    o << "MachGuiBuildProgressBar " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiBuildProgressBar " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

float MachGuiBuildProgressBar::percentageComplete() const
{
    return percentageComplete_;
}

/* End BUILDBAR.CPP *************************************************/
