/*
 * P R O M T E X T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPromptText

    Implements the prompt text displayed in the prompt text area of the in-game screen.
*/

#ifndef _MACHGUI_PROMTEXT_HPP
#define _MACHGUI_PROMTEXT_HPP

#include "base/base.hpp"
#include "gui/Displayable.hpp"
#include "gui/gui.hpp"
#include "gui/EditBox.hpp"
#include "gui/Font.hpp"
#include "machphys/machphys.hpp"

#include <string>
#include <vector>

class MachCameras;
class MachInGameScreen;

namespace System
{

class IConsole;

} // namespace System

// orthodox canonical (revoked)
class MachPromptText : public GuiSingleLineEditBox
{
public:
    // ctor. Owner is pParent. Area covered is bounded by relativeBoundary.
    MachPromptText(
        MachInGameScreen* pParent,
        const Gui::Boundary& relativeBoundary,
        MachCameras*,
        GuiDisplayable* pPassEventsTo);

    // dtor.
    ~MachPromptText() override;

    void setConsole(System::IConsole* pConsole);

    // Set/get the prompt text related to cursor position
    void setCursorPromptText(const std::string& prompt);
    void setCursorPromptText(const std::string& prompt, bool restartScroll);
    const std::string& cursorPromptText() const;

    // Clear any prompt text associated with the cursor position
    void clearCursorPromptText();

    // Set/get the prompt text related to current command state
    void setCommandPromptText(const std::string& prompt);
    const std::string& commandPromptText() const;

    // Clear any prompt text associated with the current command state
    void clearCommandPromptText();

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPromptText& t);

    enum PromptDisplayed
    {
        CURSORTEXT,
        COMMANDTEXT,
        NOTEXT
    };

    bool doHandleKeyEvent(const GuiKeyEvent& event) override;

protected:
    // Inherited from GuiDisplayable...
    void doDisplay() override;

    // virtual void doHandleContainsMouseEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseClickEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseEnterEvent( const GuiMouseEvent& rel );
    // virtual void doHandleMouseExitEvent( const GuiMouseEvent& rel );

    bool doHandleCharEvent(const GuiCharEvent& event) override;

    void displayChatMessage();
    void displayPromptText(PromptDisplayed textType, const std::vector<std::string>& textLines);

    int maxWidth() const override;
    Gui::Coord getPromptTextAbsolutePosition() const;

    void submit();

    // Pass mouse position in as relative to this GuiDisplayable. Updates
    // mouse position to be relative to pPassEventsTo_.
    // bool passEventTo( GuiMouseEvent* );

    bool processesMouseEvents() const override;

private:
    // Operations deliberately revoked
    MachPromptText(const MachPromptText&);
    MachPromptText& operator=(const MachPromptText&);
    bool operator==(const MachPromptText&);

    // Data members...
    GuiBitmap promptBmp_;
    std::string cursorPromptText_; // The prompt displayed for mouse moves
    std::string commandPromptText_; // The prompt displayed for current command
    std::vector<std::string> cursorPromptTextLines_;
    std::vector<std::string> commandPromptTextLines_;
    MachCameras* pCameras_{};
    bool displayCursorPromptText_{};
    PromptDisplayed promptDisplayed_ = NOTEXT;
    // The font the prompt text is drawn in.
    GuiBmpFont promptFont_;
    GuiBmpFont shadowFont_;
    int blitToX_{};
    bool refresh_ = true;
    bool restartScroll_{};
    GuiBitmap lightOn_;
    GuiBitmap lightOff_;
    bool enteringChatMessage_{};
    MachPhys::Race chatMessageIntendedForRace_ = MachPhys::N_RACES;
    std::string chatMessageIntendedForStr_;
    std::string lastDisplayedChatMessage_;
    int beginningTextWidth_{};
    GuiDisplayable* pPassEventsTo_{};
    int standardMessageIndex_{};
    int opponentIndex_{};
    MachInGameScreen* pInGameScreen_{};
    System::IConsole* pConsole_{};
};

#endif

/* End PROMTEXT.HPP *************************************************/
