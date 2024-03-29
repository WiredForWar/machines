/*
 * C H C K B O X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCheckBox

    Check box class.
*/

#ifndef _MACHGUI_CHCKBOX_HPP
#define _MACHGUI_CHCKBOX_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/gui.hpp"
#include "gui/font.hpp"
#include "gui/displaya.hpp"
#include "machgui/focusctl.hpp"

class MachGuiCheckBox
    : public GuiDisplayable
    , public MachGuiFocusCapableControl
// Canonical form revoked
{
public:
    using Callback = std::function<void(MachGuiCheckBox*)>;

    // The coord specifying the postion of the box is the top left corner of the box,
    // not the text label to the right of it.
    MachGuiCheckBox(
        MachGuiStartupScreens*,
        GuiDisplayable* pParent,
        const Gui::Coord& boxTopLeft,
        const string& label,
        bool isChecked = false);
    MachGuiCheckBox(
        MachGuiStartupScreens*,
        GuiDisplayable* pParent,
        const Gui::Coord& boxTopLeft,
        uint idsLabel,
        bool isChecked = false);
    ~MachGuiCheckBox() override;

    // Check/uncheck box
    void setChecked(bool isChecked);

    // Determine if isChecked
    bool isChecked() const { return isChecked_; }

    void setCallback(Callback callback) { callback_ = callback; }

    void CLASS_INVARIANT;

    bool executeControl() override;
    void hasFocus(bool) override;

protected:
    void doDisplay() override;
    void doHandleMouseClickEvent(const GuiMouseEvent&) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCheckBox& t);

    MachGuiCheckBox(const MachGuiCheckBox&);
    MachGuiCheckBox& operator=(const MachGuiCheckBox&);

    string label_;
    bool isChecked_;
    bool isHighlighted_;
    Callback callback_;
    GuiBmpFont font_;
    GuiBitmap checkBmp_;
    GuiBitmap uncheckBmp_;
    GuiBitmap checkFocusBmp_;
    GuiBitmap uncheckFocusBmp_;
    GuiBitmap checkHighlightBmp_;
    GuiBitmap uncheckHighlightBmp_;
};

#endif

/* End CHCKBOX.HPP **************************************************/
