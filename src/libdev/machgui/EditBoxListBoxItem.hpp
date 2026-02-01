/*
 * E D I T L I S T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiEditBoxListBoxItem

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_EDITLIST_HPP
#define _MACHGUI_EDITLIST_HPP

#include "base/base.hpp"
#include "machgui/SingleSelectionListBoxItem.hpp"

class GuiSingleLineEditBox;

class MachGuiEditBoxListBoxItem : public MachGuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    using Callback = std::function<void()>;

    MachGuiEditBoxListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pListBox,
        size_t width,
        const std::string& text);
    ~MachGuiEditBoxListBoxItem() override;

    void CLASS_INVARIANT;

    // Call every frame to get the edit box caret to flash.
    void update();

    // Get the last inputText that was entered into the edit box.
    std::string inputText() const;

    // Set the setInputText displayed in the edit box.
    void setInputText(const std::string& str);
    void setInputTextChangedCallback(Callback callback);

    // Set max chars that can be entered into edit box. zero = no limit
    size_t maxChars() const;
    void maxChars(size_t);

protected:
    void select() override;
    void unselect() override;

    GuiSingleLineEditBox* singleLineEditBox();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiEditBoxListBoxItem& t);

    MachGuiEditBoxListBoxItem(const MachGuiEditBoxListBoxItem&);
    MachGuiEditBoxListBoxItem& operator=(const MachGuiEditBoxListBoxItem&);

    // Data members...
    GuiSingleLineEditBox* pSingleLineEditBox_{};
    std::string inputText_;
    size_t maxChars_;
    Callback textChangedCallback_{};
};

#endif

/* End EDITLIST.HPP *************************************************/
