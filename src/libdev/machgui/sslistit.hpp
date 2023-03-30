/*
 * S S L I S T I T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiSingleSelectionListBoxItem

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_SSLISTIT_HPP
#define _MACHGUI_SSLISTIT_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "gui/root.hpp"
#include "gui/sslistit.hpp"

class GuiBmpFont;
class MachGuiStartupScreens;
class MachGuiSingleSelectionListBox;

class MachGuiSingleSelectionListBoxItem : public GuiSingleSelectionListBoxItem
// Canonical form revoked
{
public:
    // TODO: Find a suitable abstraction for getting handle to game state ("startupData" lol) so MGSS dependency can be
    // removed
    MachGuiSingleSelectionListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        MachGuiSingleSelectionListBox* pParentListBox,
        size_t width,
        const string& text);

    ~MachGuiSingleSelectionListBoxItem() override;

    void CLASS_INVARIANT;

    static size_t reqHeight();

protected:
    // This variant utilized by MachGuiDropDownListBoxItem to initialize it. Hence, pMyListBox_ is null
    // TODO: Find a suitable abstraction for getting handle to game state ("startupData" lol) so MGSS dependency can be
    // removed
    MachGuiSingleSelectionListBoxItem(
        MachGuiStartupScreens* pStartupScreens,
        GuiSingleSelectionListBox* pParentListBox,
        size_t width,
        const string& text);

    void select() override;
    void unselect() override;

    static GuiBmpFont getFont();
    static GuiBmpFont getUnderlineFont();
    static GuiBmpFont getHighlightFont();

    void doDisplay() override;

    bool highlighted() const;
    const string& text() const;
    // TODO: Eliminate this. Subclasses in declared in ctxjoin.cpp and ctxmulti.cpp use this to join & name the MP game.
    DECL_DEPRECATED MachGuiStartupScreens* startupScreens();
    MachGuiSingleSelectionListBox* myListBox();

    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiSingleSelectionListBoxItem& t);

    MachGuiSingleSelectionListBoxItem(const MachGuiSingleSelectionListBoxItem&);
    MachGuiSingleSelectionListBoxItem& operator=(const MachGuiSingleSelectionListBoxItem&);

    string text_;
    bool highlighted_;
    // TODO: Eliminate this. Subclasses in declared in ctxjoin.cpp and ctxmulti.cpp use this to join & name the MP game.
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
    MachGuiSingleSelectionListBox* pMyListBox_;

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;
};

#endif

/* End SSLISTIT.HPP *************************************************/
