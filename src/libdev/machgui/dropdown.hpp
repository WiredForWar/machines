/*
 * D R O P D O W N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiDropDownListBox

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_DROPDOWN_HPP
#define _MACHGUI_DROPDOWN_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "machgui/sslistit.hpp"
#include "machgui/sslistbx.hpp"
#include "machgui/autodel.hpp"

class MachGuiDropDownListBoxItem;

class MachGuiDropDownListBox
    : public GuiSingleSelectionListBox
    , public MachGuiAutoDeleteDisplayable
{
public:
    MachGuiDropDownListBox(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText);

    MachGuiDropDownListBox(
        MachGuiStartupScreens* pParent,
        const Gui::Box& box,
        size_t horizontalSpacing,
        size_t verticalSpacing,
        size_t scrollInc,
        size_t itemWidth,
        const ctl_vector<std::string>& itemText,
        bool whiteFont);

    ~MachGuiDropDownListBox() override;

    bool containsMousePointer() override;
    void update() override;

protected:
    void doDisplay() override;

    virtual void itemSelected(const std::string& text) = 0;

    virtual MachGuiDropDownListBoxItem* createListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiDropDownListBox* pListBox,
        size_t width,
        const std::string& text,
        bool whiteFont);
    friend class MachGuiDropDownListBoxItem;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDropDownListBox& t);
    void CLASS_INVARIANT;

    // Data members...
    ctl_vector<std::string> itemText_;
    size_t itemWidth_;
    double timeInterval_;
    double timeStart_;
    size_t nextItem_;
    bool whiteFont_;
};

class MachGuiDropDownListBoxItem : public MachGuiSingleSelectionListBoxItem
{
public:
    MachGuiDropDownListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiDropDownListBox* pListBox,
        size_t width,
        const std::string& text);

    MachGuiDropDownListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiDropDownListBox* pListBox,
        size_t width,
        const std::string& text,
        bool whiteFont);

protected:
    void doDisplay() override;

    void select() override;

    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;

    static GuiBmpFont getWhiteFont();

private:
    // Data members...
    MachGuiDropDownListBox* pListBox_;
    bool whiteFont_;
};

#endif

/* End DROPDOWN.HPP *************************************************/
