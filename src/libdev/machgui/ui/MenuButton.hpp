/*
 * M E N U B U T N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMenuButton

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MENUBUTN_HPP
#define _MACHGUI_MENUBUTN_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"
#include "machgui/startup.hpp"
#include "machgui/focusctl.hpp"

class MachGuiMenuButton
    : public GuiDisplayable
    , public MachGuiFocusCapableControl
// Canonical form revoked
{
public:
    // TODO: Eliminate entirely MachGuiStartupScreens from this constructor
    MachGuiMenuButton(
        GuiRoot* pRootParent,
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        unsigned int stringId,
        MachGui::ButtonEvent buttonEvent);
    // TODO: Eliminate entirely MachGuiStartupScreens from this constructor.
    MachGuiMenuButton(
        MachGuiStartupScreens* pStartupScreens,
        const Gui::Box& box,
        unsigned int stringId,
        MachGui::ButtonEvent,
        GuiDisplayable* pParent);
    ~MachGuiMenuButton() override;

    void CLASS_INVARIANT;

    // Disable/Enable the button.
    void disabled(bool);

    bool highlighted() const;
    uint stringId() const;

    // Inherited from MachGuiFocusCapableControl...
    bool isFocusEnabled() const override;

    bool executeControl() override;
    // PRE( hasFocus() );
    // PRE( not isDisabled() );

    void hasFocus(bool) override;

    void setMsgBoxButton(bool val) { msgBoxButton_ = val; };

protected:
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doDisplay() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiMenuButton& t);

    MachGuiMenuButton(const MachGuiMenuButton&);
    MachGuiMenuButton& operator=(const MachGuiMenuButton&);

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;

    // TODO: Eliminate dependency. Still required for buttonAction and other GUI event handling
    // Note: The TODO is blocked by MachGuiFocusCapableControl
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;

    unsigned int stringId_;
    bool highlighted_;
    bool flash_;
    bool disabled_;
    bool msgBoxButton_;
    MachGui::ButtonEvent buttonEvent_;
};

#endif

/* End MENUBUTN.HPP *************************************************/
