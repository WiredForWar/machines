/*
 * H E A L T H . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "machgui/health.hpp"
#include "machgui/gui.hpp"
#include "gui/manager.hpp"
#include "gui/painter.hpp"

//////////////////////////////////////////////////////////////////////

MachGuiHealthBar::MachGuiHealthBar(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    unsigned width,
    MachPhys::HitPointUnits maxHits,
    MachPhys::ArmourUnits maxArmour)
    : GuiDisplayable(pParent, Gui::Box(rel, width, healthBarHeight()))
    , maxHits_(maxHits)
    , maxArmour_(maxArmour)
    , currentHits_(0)
    , currentArmour_(0)
{
    PRE(maxHits > 0);
    PRE(maxArmour > 0);
}

MachGuiHealthBar::~MachGuiHealthBar()
{
    // Intentionally Empty
}

//////////////////////////////////////////////////////////////////////

MachPhys::HitPointUnits MachGuiHealthBar::maxHits() const
{
    return maxHits_;
}

MachPhys::ArmourUnits MachGuiHealthBar::maxArmour() const
{
    return maxArmour_;
}

void MachGuiHealthBar::hp(MachPhys::HitPointUnits hits)
{
    PRE_INFO(hits);
    PRE_INFO(maxHits());
    PRE(hits <= maxHits());

    unsigned interiorWidth = width() - (2 * MachGui::barBorderThickness());
    unsigned oldWidth = (currentHits_ * interiorWidth) / maxHits_;
    unsigned newWidth = (hits * interiorWidth) / maxHits_;

    if (newWidth != oldWidth)
        changed();

    currentHits_ = hits;
}

void MachGuiHealthBar::armour(MachPhys::ArmourUnits armour)
{
    PRE_INFO(armour);
    PRE_INFO(maxArmour());
    PRE(armour <= maxArmour());

    unsigned interiorWidth = width() - (2 * MachGui::barBorderThickness());
    unsigned oldWidth = (currentArmour_ * interiorWidth) / maxArmour_;
    unsigned newWidth = (armour * interiorWidth) / maxArmour_;

    if (newWidth != oldWidth)
        changed();

    currentArmour_ = armour;
}

//////////////////////////////////////////////////////////////////////

// static
Gui::Colour MachGuiHealthBar::hpColour(unsigned currentValue, unsigned maxValue)
// width in pixels for a given ratio
{
    PRE(currentValue <= maxValue);
    const float LOW_THRESHOLD = 1.0 / 3.0;
    const float MID_THRESHOLD = 1.7 / 3.0;

    Gui::Colour result = Gui::Colour(0, 128.0 / 255.0, 0);
    float ratio = currentValue / (float)maxValue;
    if (ratio <= LOW_THRESHOLD)
        result = Gui::RED();
    else if (ratio <= MID_THRESHOLD)
        result = Gui::YELLOW();

    return result;
}

// static
Gui::Colour MachGuiHealthBar::armourColour(unsigned currentValue, unsigned maxValue)
// width in pixels for a given ratio
{
    PRE(currentValue <= maxValue);
    const float LOW_THRESHOLD = 1.0 / 3.0;
    const float MID_THRESHOLD = 1.7 / 3.0;

    static const Gui::Colour blue(33.0 / 255.0, 6.0 / 255.0, 217.0 / 255.0);
    static const Gui::Colour purple(128.0 / 255.0, 0, 128.0 / 255.0);
    static const Gui::Colour red(252.0 / 255.0, 0.0, 1.0 / 255.0);

    Gui::Colour result = blue;
    float ratio = currentValue / (float)maxValue;
    if (ratio <= LOW_THRESHOLD)
        result = red;
    else if (ratio <= MID_THRESHOLD)
        result = purple;

    return result;
}

//////////////////////////////////////////////////////////////////////

void MachGuiHealthBar::doDisplay()
{
    const int borderThickness = MachGui::barBorderThickness();
    const int shadowThickness = MachGui::barShadowThickness();
    const int dividerThickness = MachGui::barDividerThickness();
    const int barOffset = MachGui::barValueLineOffset();
    const int barThickness = MachGui::barValueLineThickness();

    const double hpRatio = static_cast<double>(currentHits_) / maxHits_;
    const double armorRatio = static_cast<double>(currentArmour_) / maxArmour_;

    const unsigned interiorWidth = width() - (2 * borderThickness) - shadowThickness;
    const unsigned hitsWidth = hpRatio * interiorWidth;
    const unsigned armourWidth = armorRatio * interiorWidth;

    Gui::Coord topLeft(absoluteCoord().x() + borderThickness, absoluteCoord().y() + borderThickness);
    Gui::Coord sc(absoluteCoord().x() + borderThickness + shadowThickness, absoluteCoord().y() + borderThickness);
    Gui::Coord ac(
        absoluteCoord().x() + borderThickness + shadowThickness,
        absoluteCoord().y() + barOffset + borderThickness + shadowThickness);
    Gui::Coord dc(
        absoluteCoord().x() + borderThickness + shadowThickness,
        absoluteCoord().y() + borderThickness + shadowThickness + barThickness);
    Gui::Coord hc(
        absoluteCoord().x() + borderThickness + shadowThickness,
        absoluteCoord().y() + barOffset + borderThickness + shadowThickness + barThickness + dividerThickness);

    GuiPainter::instance().horizontalLine(sc, interiorWidth, Gui::BLACK(), shadowThickness);
    GuiPainter::instance().horizontalLine(ac, interiorWidth - 1, Gui::LIGHTGREY(), barThickness);
    GuiPainter::instance().horizontalLine(ac, armourWidth, armourColour(currentArmour_, maxArmour_), barThickness);
    GuiPainter::instance().horizontalLine(dc, interiorWidth, Gui::LIGHTGREY(), dividerThickness);
    GuiPainter::instance().horizontalLine(hc, interiorWidth - 1, Gui::LIGHTGREY(), barThickness);
    GuiPainter::instance().horizontalLine(hc, hitsWidth, hpColour(currentHits_, maxHits_), barThickness);
    GuiPainter::instance().verticalLine(
        topLeft,
        shadowThickness + barThickness + dividerThickness + barThickness,
        Gui::BLACK(),
        shadowThickness);

    GuiPainter::instance().hollowRectangle(absoluteBoundary(), Gui::WHITE(), borderThickness);
}

// static
size_t MachGuiHealthBar::healthBarHeight()
{
    return MachGui::barBorderThickness() + MachGui::barShadowThickness() + MachGui::barValueLineThickness()
        + MachGui::barDividerThickness() + MachGui::barValueLineThickness() + MachGui::barBorderThickness();
}

void MachGuiHealthBar::depress(bool doDepress)
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

std::ostream& operator<<(std::ostream& o, const MachGuiHealthBar& t)
{
    o << "MachGuiHealthBar " << (void*)&t << " start" << std::endl;
    o << "MachGuiHealthBar " << (void*)&t << " end" << std::endl;

    return o;
}

//////////////////////////////////////////////////////////////////////

/* End HEALTH.CPP ***************************************************/
