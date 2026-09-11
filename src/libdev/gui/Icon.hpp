
#ifndef _GUI_ICON_HPP
#define _GUI_ICON_HPP

#include "gui/gui.hpp"
#include "gui/Button.hpp"

#include "system/PathName.hpp"

/* //////////////////////////////////////////////////////////////// */

class GuiIcon : public GuiBitmapButtonWithFilledBorder
{
public:
    GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const std::pair<SysPathName, SysPathName>& bitmap);
    GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const SysPathName& bmp);
    GuiIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const GuiBitmap& bitmap);

    // The room an icon showing this bitmap takes up: the bitmap itself plus the
    // border drawn around it. A layout that has to leave space for an icon asks
    // this before making one.
    static Gui::Size sizeFor(const GuiBitmap& bitmap);

protected:
    ~GuiIcon() override;

private:
    static GuiBorderMetrics iconBorderMetrics();

    GuiIcon(const GuiIcon&) = delete;
    GuiIcon& operator=(const GuiIcon&) = delete;
    bool operator==(const GuiIcon&) const = delete;
};

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _GUI_ICON_HPP
