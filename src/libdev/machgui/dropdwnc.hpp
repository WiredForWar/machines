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
#include "ctl/vector.hpp"
#include "gui/displaya.hpp"
#include "gui/restring.hpp"
#include "gui/root.hpp"
#include "machgui/dropdown.hpp"

class MachGuiStartupScreens;
class GuiBmpFont;
class MachGuiDropDownList;

class MachGuiDropDownListBoxCreator
    : public GuiDisplayable
    , public MachGuiFocusCapableControl
// Canonical form revoked
{
public:
    using DropDownListBoxItem = const void*;
    using DropDownListBoxItems = ctl_vector<DropDownListBoxItem>;

    template <typename T, int Size> static DropDownListBoxItems createBoxItems(T (&Values)[Size])
    {
        MachGuiDropDownListBoxCreator::DropDownListBoxItems items;
        items.reserve(Size);
        for (const T& Value : Values)
        {
            items.push_back(&Value);
        }
        return items;
    };

    // TODO: Eliminate entirely MachGuiStartupScreens from these constructors. Focus capable control stuff is what MGSS
    // still needed for
    MachGuiDropDownListBoxCreator(
        GuiDisplayable* pParent,
        MachGuiStartupScreens*,
        int width,
        const GuiStrings& availText);
    MachGuiDropDownListBoxCreator(
        GuiDisplayable* pParent,
        MachGuiStartupScreens*,
        int width,
        const GuiStrings& availText,
        bool whiteFont);
    MachGuiDropDownListBoxCreator(
        GuiDisplayable* pParent,
        MachGuiStartupScreens*,
        const Gui::Coord& relCoord,
        int width,
        const GuiStrings& availText,
        bool whiteFont,
        bool border);
    ~MachGuiDropDownListBoxCreator() override;

    void CLASS_INVARIANT;

    // Get and set the text shown in the control.
    const string& text() const;
    void text(const string&);

    // Get ptr to item associated with currently highlighted text
    const DropDownListBoxItem item() const;
    // PRE ( hasItems() )

    bool setCurrentItem(const DropDownListBoxItem item);

    // Establish if there are values associated with drop down list box entries
    bool hasItems() const;

    const DropDownListBoxItems& items() const;

    // Set item data associated with list box item text
    void items(const DropDownListBoxItems& items);

    void clear();

    // Get minimum height required for the gui item.
    static size_t reqHeight(bool border = false);

    bool doHandleNavigationKey(NavKey, MachGuiFocusCapableControl**) override;
    void hasFocus(bool) override;

protected:
    static GuiBmpFont getFont();
    static GuiBmpFont getWhiteFont();
    static GuiBmpFont getHighlightFont();

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
        const ctl_vector<string>& itemText,
        MachGuiDropDownListBoxCreator*);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDropDownListBoxCreator& t);

    MachGuiDropDownListBoxCreator(const MachGuiDropDownListBoxCreator&);
    MachGuiDropDownListBoxCreator& operator=(const MachGuiDropDownListBoxCreator&);

    // Data members...
    bool hovered_ = false;
    GuiStrings strings_;
    GuiString text_;
    DropDownListBoxItems items_;
    DropDownListBoxItem item_;
    // TODO: Remove this once the focus capable control and auto-delete displayables refactor is done...
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
    bool whiteFont_;
    bool border_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
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
        const ctl_vector<string>& itemText,
        MachGuiDropDownListBoxCreator*);

    MachGuiDropDownList(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<string>& itemText,
        MachGuiDropDownListBoxCreator*,
        bool whiteFont);

    void itemSelected(const string& text) override;

private:
    MachGuiDropDownListBoxCreator* pCreator_;
};

#endif

/* End DROPDWNC.HPP *************************************************/
