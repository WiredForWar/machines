
#include "gui/Icon.hpp"

//////////////////////////////////////////////////////////////////////

GuiIcon::GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const std::pair<SysPathName, SysPathName>& bmps)
    : GuiIcon(pParent, rel, Gui::bitmap(bmps.first))
{
}

GuiIcon::GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const SysPathName& bmp)
    : GuiIcon(pParent, rel, Gui::bitmap(bmp))
{
}

GuiIcon::GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const GuiBitmap& bitmap)
    : GuiBitmapButtonWithFilledBorder(
        pParent,
        rel,
        iconBorderMetrics(),
        GuiFilledBorderColours(Gui::BLACK(), Gui::LIGHTGREY(), Gui::DARKGREY(), Gui::RED()),
        bitmap,
        Gui::Coord(1, 1) * Gui::uiScaleFactor())
{
}

GuiIcon::~GuiIcon() = default;

Gui::Size GuiIcon::sizeFor(const GuiBitmap& bitmap)
{
    const GuiBorderMetrics m = iconBorderMetrics();

    return Gui::Size(m.totalHorizontalThickness() + bitmap.width(), m.totalVerticalThickness() + bitmap.height());
}

GuiBorderMetrics GuiIcon::iconBorderMetrics()
{
    return GuiBorderMetrics(1, 1, 1) * Gui::uiScaleFactor();
}
