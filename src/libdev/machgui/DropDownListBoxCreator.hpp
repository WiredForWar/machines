/*
 * D R O P D W N C . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiDropDownListBoxCreator

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_DROPDWNC_HPP
#define _MACHGUI_DROPDWNC_HPP

#include "base/base.hpp"
#include "ctl/Vector.hpp"
#include "gui/Displayable.hpp"
#include "gui/ResourceString.hpp"
#include "gui/Root.hpp"
#include "machgui/DropDownListBox.hpp"

#include <functional>

class MachGuiStartupScreens;
// GuiBmpFont is a using alias for Ren::BmpFont, declared in gui/gui.hpp
class MachGuiDropDownList;

class MachGuiDropDownListBoxCreator
    : public GuiDisplayable
// Canonical form revoked
{
public:
    using Callback = std::function<void()>;

    // TODO: Eliminate entirely MachGuiStartupScreens from these constructors. Focus capable control stuff is what MGSS
    // still needed for
    MachGuiDropDownListBoxCreator(GuiDisplayable* pParent, MachGuiStartupScreens*, int width);
    MachGuiDropDownListBoxCreator(GuiDisplayable* pParent, MachGuiStartupScreens*, int width, bool whiteFont);
    MachGuiDropDownListBoxCreator(
        GuiDisplayable* pParent,
        MachGuiStartupScreens*,
        const Gui::Coord& relCoord,
        int width,
        bool whiteFont,
        bool border);
    ~MachGuiDropDownListBoxCreator() override;

    void CLASS_INVARIANT;

    std::string currentText() const;
    void setCurrentText(const std::string&);

    // Which entry is selected, or -1 when none is. Whatever the entries stand for
    // is the caller's to keep, in the order the text was given in.
    int currentIndex() const;
    void setCurrentIndex(int index);

    void setAvailText(const GuiStrings& availText);

    // Get minimum height required for the gui item.
    static size_t reqHeight();

    bool doHandleNavigationKey(NavKey, GuiDisplayable**) override;
    void hasFocus(bool) override;

    // A disabled drop down shows its selection dimmed, and neither opens nor takes
    // the focus. Use it for a choice that the state of another control has settled.
    bool isFocusEnabled() const override;

    void setCurrentIndexChangedCallback(Callback callback);

protected:
    static GuiBmpFont getFont();
    static GuiBmpFont getWhiteFont();
    static GuiBmpFont getHighlightFont();
    static GuiBmpFont getDisabledFont();

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;

    void doDisplay() override;

    virtual MachGuiDropDownList* createDropDownList(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText,
        MachGuiDropDownListBoxCreator*);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDropDownListBoxCreator& t);

    MachGuiDropDownListBoxCreator(const MachGuiDropDownListBoxCreator&) = delete;
    MachGuiDropDownListBoxCreator& operator=(const MachGuiDropDownListBoxCreator&) = delete;

    Callback currentIndexChangedCallback_;

    // Data members...
    bool hovered_ = false;
    GuiStrings strings_;
    int currentIndex_{-1};
    // TODO: Remove this once the focus capable control and auto-delete displayables refactor is done...
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
    bool whiteFont_;
    bool border_;
};

class MachGuiDropDownList : public MachGuiDropDownListBox
{
public:
    // TODO: Eliminate entirely MachGuiStartupScreens from these constructors. Focus capable control & auto-delete stuff
    // is what MGSS still needed for
    MachGuiDropDownList(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText,
        MachGuiDropDownListBoxCreator*);

    MachGuiDropDownList(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText,
        MachGuiDropDownListBoxCreator*,
        bool whiteFont);

    void itemSelected(const std::string& text) override;

private:
    MachGuiDropDownListBoxCreator* pCreator_;
};

#endif

/* End DROPDWNC.HPP *************************************************/
