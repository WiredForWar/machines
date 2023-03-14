
#include "gui/icon.hpp"

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
        GuiBorderMetrics(1, 1, 1),
        GuiFilledBorderColours(Gui::BLACK(), Gui::LIGHTGREY(), Gui::DARKGREY(), Gui::RED()),
        bitmap,
        Gui::Coord(1, 1))
{
}

GuiIcon::~GuiIcon() = default;
