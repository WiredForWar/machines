/*
 * M A N A G E R . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_MANAGER_HPP
#define _GUI_MANAGER_HPP

#include "gui/Event.hpp"
#include "gui/Root.hpp"

//////////////////////////////////////////////////////////////////////
class GuiDisplayable;

template <class T> class ctl_list;

class GuiColourScheme
{
public:
    static const GuiColourScheme& defaultScheme();

    ///////////////////////////////

    const Gui::Colour& mainBack() const;
    const Gui::Colour& mainInk() const;

    const Gui::Colour& borderBack() const;
    const Gui::Colour& borderLo() const;
    const Gui::Colour& borderHi() const;

    const Gui::Colour& headingBack() const;
    const Gui::Colour& headingInk() const;

    const Gui::Colour& buttonBack() const;
    const Gui::Colour& buttonInk() const;
    const Gui::Colour& buttonLo() const;
    const Gui::Colour& buttonHi() const;
    const Gui::Colour& buttonDefault() const;

    ///////////////////////////////

private:
    GuiColourScheme();

    Gui::Colour mainBack_;
    Gui::Colour mainInk_;
    Gui::Colour borderBack_;
    Gui::Colour borderLo_;
    Gui::Colour borderHi_;

    Gui::Colour headingBack_;
    Gui::Colour headingInk_;

    Gui::Colour buttonBack_;
    Gui::Colour buttonInk_;
    Gui::Colour buttonLo_;
    Gui::Colour buttonHi_;
    Gui::Colour buttonDefault_;
};

//////////////////////////////////////////////////////////////////////

class GuiManager
// Singleton
{
public:
    static GuiManager& instance();
    ~GuiManager();

    const GuiColourScheme& colourScheme() const;

    // Get GuiDisplayables to respond to keyboard and mouse events. As a result of this
    // a GuiDisplayable may indicate that it needs redrawing.
    bool update();

    // Redraw any GuiDisplayables modified during the update phase.
    void display();

    bool hasRoot() const;

    const GuiRoot& root() const;
    // PRE( hasRoot() );

    void changeRoot(GuiRoot* pNewRoot);

    bool keyboardFocusExists() const;

    GuiDisplayable& keyboardFocus();
    // PRE( keyboardFocusExists() );

    void keyboardFocus(GuiDisplayable* pNewKeyboardFocus);
    // PRE( pNewKeyboardFocus != NULL );
    // POST( keyboardFocusExists() );
    // POST( &keyboardFocus() == pNewKeyboardFocus );

    bool charFocusExists() const;

    GuiDisplayable& charFocus();
    // PRE( charFocusExists() );

    void charFocus(GuiDisplayable* pNewCharFocus);
    // PRE( pNewCharFocus != NULL );
    // POST( charFocusExists() );

    void removeCharFocus();
    // POST( not charFocusExists() );

    // The displayables that accept the keyboard focus, in the order they said so.
    // Keyboard navigation walks it, and the first to join takes the focus.
    using FocusChain = ctl_vector<GuiDisplayable*>;

    const FocusChain& focusChain() const;

    // True while the keys may be given to pDisplayable. Navigation asks this
    // rather than each displayable answering for itself, so that being disabled
    // means the same thing everywhere.
    //
    // Being disabled keeps the keys away, and nothing else: a disabled
    // displayable is still handed the mouse, still occupies its boundary, and
    // still draws. Declining to act on what arrives is its own business.
    bool canTakeFocus(const GuiDisplayable* pDisplayable) const;
    // PRE( pDisplayable != nullptr );

private:
    GuiManager(const GuiManager&) = delete;
    bool operator==(const GuiManager&) const = delete;
    GuiManager& operator=(const GuiManager&) = delete;

    // Called by the framework when pDisplayable is being created
    void isBeingCreated(GuiDisplayable* pDisplayable);
    // Called by the framework when pDisplayable is being destructed
    void isBeingDeleted(GuiDisplayable* pDisplayable);

    GuiRoot& root();
    // PRE( hasRoot() );

    GuiManager();

    void processEvents();
    void processMouseEvent(const GuiMouseEvent&);
    void updateMouseFocus();

    static Gui::ScrollState getScrollDirection(DevButtonEvent::Action act);

    static constexpr int N_BUFFERS = 2;

    void addToFocusChain(GuiDisplayable* pDisplayable);
    void removeFromFocusChain(GuiDisplayable* pDisplayable);

    // Data members
    GuiColourScheme colourScheme_ = GuiColourScheme::defaultScheme();
    GuiRoot* pRoot_{};
    GuiDisplayable* pMouseFocus_{};
    GuiDisplayable* pKeyboardFocus_{};
    GuiDisplayable* pCharacterFocus_{};
    FocusChain focusChain_{};

    friend class GuiDisplayable;
    friend void GuiRoot::becomeRoot();
};

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _GUI_MANAGER_HPP

/* End MANAGER.HPP **************************************************/
