/*
 * C H A T W I N D . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCharWindow

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CHATWIND_HPP
#define _MACHGUI_CHATWIND_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"
#include "gui/root.hpp"

class MachGuiStartupScreens;

class MachGuiChatWindow : public GuiDisplayable
// Canonical form revoked
{
public:
    // TODO: Eliminate entirely MachGuiStartupScreens from this constructor
    MachGuiChatWindow(GuiRoot* pRootParent, MachGuiStartupScreens* pStartupScreens, const Gui::Box& box);
    ~MachGuiChatWindow() override;

    static void addText(const std::string& text);

    static void clearAllText();

    void CLASS_INVARIANT;

protected:
    void doDisplay() override;

    using strings = std::vector<std::string>;

    static size_t& numLines();
    static strings& linesOfText();
    static size_t& chatWidth();

    static MachGuiChatWindow*& chatWindow();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiChatWindow& t);

    MachGuiChatWindow(const MachGuiChatWindow&);
    MachGuiChatWindow& operator=(const MachGuiChatWindow&);

    // Data members...

    // A GuiRoot such as MachGuiStartupScreens
    GuiRoot* pRootParent_;

    // TODO: Eliminate dependency. Still required for message box stuff >.<
    DECL_DEPRECATED MachGuiStartupScreens* pStartupScreens_;
};

#endif

/* End CHATWIND.HPP *************************************************/
