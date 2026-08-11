/*
 * M E N U B U T N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMenuButton

    A brief description of the class should go in here
*/

#pragma once

#include "gui/Displayable.hpp"

class GuiRoot;
class MachGuiStartupScreens;

namespace MachGui
{

enum class ButtonEvent;

} // namespace MachGui

class MachGuiMenuButton
    : public GuiDisplayable
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

    bool highlighted() const;
    uint stringId() const;

    // Inherited from GuiDisplayable...
    bool executeControl() override;
    // PRE( hasFocus() );
    // PRE( isEnabled() );

    void hasFocus(bool) override;

    void setMsgBoxButton(bool val) { msgBoxButton_ = val; };

protected:
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doDisplay() override;

    // A button being switched off is not being pressed or pointed at any more.
    void doEnabledChanged() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiMenuButton& t);

    MachGuiMenuButton(const MachGuiMenuButton&) = delete;
    MachGuiMenuButton& operator=(const MachGuiMenuButton&) = delete;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;

    // TODO: Eliminate dependency. Still required to report the button event.
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;

    unsigned int stringId_{};
    bool highlighted_{};
    bool flash_{};
    bool msgBoxButton_{};
    MachGui::ButtonEvent buttonEvent_;
};
