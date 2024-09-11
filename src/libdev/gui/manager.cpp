// #include <windows.h>
#include <algorithm>
#include <functional>

// #include "ctl/algorith.hpp"
#include "ctl/list.hpp"
#include "device/eventq.hpp"
#include "gui/internal/managei.hpp"
#include "gui/manager.hpp"
#include "gui/displaya.hpp"
#include "gui/event.hpp"
#include "gui/mouse.hpp"
#include "gui/painter.hpp"
#include "render/device.hpp"
#include "render/display.hpp"

#ifndef PRODUCTION
#include "world4d/scenemgr.hpp"
#include "world4d/manager.hpp"
#endif

#define CB_GUIMANAGER_DEPIMPL()                                                                                        \
    CB_DEPIMPL(GuiColourScheme, colourScheme_);                                                                        \
    CB_DEPIMPL(GuiRoot*, pRoot_);                                                                                      \
    CB_DEPIMPL(GuiDisplayable*, pMouseFocus_);                                                                         \
    CB_DEPIMPL(GuiDisplayable*, pKeyboardFocus_);                                                                      \
    CB_DEPIMPL(GuiDisplayable*, pCharacterFocus_);

//////////////////////////////////////////////////////////////////////

GuiManager& GuiManager::instance()
{
    static GuiManager instance_;
    return instance_;
}

//////////////////////////////////////////////////////////////////////

static constexpr DevEventQueue::ScanCode ScanCodes[] {
    DevKey::UP_ARROW_PAD,
    DevKey::DOWN_ARROW_PAD,
    DevKey::LEFT_ARROW_PAD,
    DevKey::RIGHT_ARROW_PAD,
    DevKey::ENTER_PAD,
    DevKey::INSERT_PAD,
    DevKey::DELETE_PAD,
    DevKey::HOME_PAD,
    DevKey::END_PAD,
    DevKey::PAGE_UP_PAD,
    DevKey::PAGE_DOWN_PAD,
    DevKey::FIVE_PAD,
    DevKey::ASTERISK_PAD,
    DevKey::FORWARD_SLASH_PAD,
    DevKey::PLUS_PAD,
    DevKey::MINUS_PAD,
    DevKey::PAD_0,
    DevKey::PAD_1,
    DevKey::PAD_2,
    DevKey::PAD_3,
    DevKey::PAD_4,
    DevKey::PAD_5,
    DevKey::PAD_6,
    DevKey::PAD_7,
    DevKey::PAD_8,
    DevKey::PAD_9,
    DevKey::SCROLL_LOCK,
    DevKey::BREAK,
    DevKey::RIGHT_CONTROL,
    DevKey::LEFT_CONTROL,
    DevKey::UP_ARROW,
    DevKey::DOWN_ARROW,
    DevKey::LEFT_ARROW,
    DevKey::RIGHT_ARROW,
    DevKey::INSERT,
    DevKey::KEY_DELETE,
    DevKey::HOME,
    DevKey::END,
    DevKey::PAGE_UP,
    DevKey::PAGE_DOWN,
    DevKey::ESCAPE,
    DevKey::ENTER,
    DevKey::SPACE,
    DevKey::BACK_SPACE,
    DevKey::TAB,
    DevKey::GRAVE,
    DevKey::LEFT_SHIFT,
    DevKey::RIGHT_SHIFT,
    DevKey::CAPS_LOCK,
    DevKey::NUM_LOCK,
    DevKey::F1,
    DevKey::F2,
    DevKey::F3,
    DevKey::F4,
    DevKey::F5,
    DevKey::F6,
    DevKey::F7,
    DevKey::F8,
    DevKey::F9,
    DevKey::F10,
    DevKey::F11,
    DevKey::F12,
    DevKey::KEY_A,
    DevKey::KEY_B,
    DevKey::KEY_C,
    DevKey::KEY_D,
    DevKey::KEY_E,
    DevKey::KEY_F,
    DevKey::KEY_G,
    DevKey::KEY_H,
    DevKey::KEY_I,
    DevKey::KEY_J,
    DevKey::KEY_K,
    DevKey::KEY_L,
    DevKey::KEY_M,
    DevKey::KEY_N,
    DevKey::KEY_O,
    DevKey::KEY_P,
    DevKey::KEY_Q,
    DevKey::KEY_R,
    DevKey::KEY_S,
    DevKey::KEY_T,
    DevKey::KEY_U,
    DevKey::KEY_V,
    DevKey::KEY_W,
    DevKey::KEY_X,
    DevKey::KEY_Y,
    DevKey::KEY_Z,
    DevKey::KEY_0,
    DevKey::KEY_1,
    DevKey::KEY_2,
    DevKey::KEY_3,
    DevKey::KEY_4,
    DevKey::KEY_5,
    DevKey::KEY_6,
    DevKey::KEY_7,
    DevKey::KEY_8,
    DevKey::KEY_9,
    DevKey::MOUSE_LEFT,
    DevKey::MOUSE_RIGHT,
    DevKey::MOUSE_MIDDLE,
    DevKey::MOUSE_EXTRA1,
    DevKey::MOUSE_EXTRA2,
    DevKey::MOUSE_EXTRA3,
    DevKey::MOUSE_EXTRA4,
    DevKey::MOUSE_EXTRA5,
    DevKey::MOUSE_EXTRA6,
    DevKey::MOUSE_EXTRA7,
    DevKey::MOUSE_EXTRA8,
};

//////////////////////////////////////////////////////////////////////

GuiManager::GuiManager()
    : pImpl_(new GuiManagerImpl(GuiColourScheme::defaultScheme()))
{
    CB_GUIMANAGER_DEPIMPL();

    colourScheme_ = GuiColourScheme::defaultScheme();
    pRoot_ = nullptr;
    pMouseFocus_ = nullptr;
    pKeyboardFocus_ = nullptr;
    pCharacterFocus_ = nullptr;

    for (DevEventQueue::ScanCode code : ScanCodes)
    {
        DevEventQueue::instance().queueEvents(code);
    }

    // Event Queue is explicity told to handle mouse wheel scrolling events
    DevEventQueue::instance().queueEvents(DevKey::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);
    DevEventQueue::instance().queueEvents(DevKey::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN);
}

GuiManager::~GuiManager()
{
    delete pImpl_;
}

//////////////////////////////////////////////////////////////////////

bool GuiManager::hasRoot() const
{
    CB_GUIMANAGER_DEPIMPL();

    return pRoot_ != nullptr;
}

const GuiRoot& GuiManager::root() const
{
    PRE(hasRoot());

    CB_GUIMANAGER_DEPIMPL();

    return *pRoot_;
}

GuiRoot& GuiManager::root()
{
    PRE(hasRoot());

    CB_GUIMANAGER_DEPIMPL();

    return *pRoot_;
}

//////////////////////////////////////////////////////////////////////

bool GuiManager::keyboardFocusExists() const
{
    CB_GUIMANAGER_DEPIMPL();

    return pKeyboardFocus_ != nullptr;
}

void GuiManager::keyboardFocus(GuiDisplayable* pNewKeyboardFocus)
{
    PRE(pNewKeyboardFocus != nullptr);

    CB_GUIMANAGER_DEPIMPL();

    pKeyboardFocus_ = pNewKeyboardFocus;
    POST(keyboardFocusExists());
    POST(&keyboardFocus() == pNewKeyboardFocus);
}

GuiDisplayable& GuiManager::keyboardFocus()
{
    PRE(keyboardFocusExists());

    CB_GUIMANAGER_DEPIMPL();

    return *pKeyboardFocus_;
}

//////////////////////////////////////////////////////////////////////

bool GuiManager::update()
{
    if (hasRoot())
    {
        processEvents();

        updateMouseFocus();

        // Call update on GuiRoot
        root().update();
    }

    bool finished = DevKeyboard::instance().keyCode(DevKey::KEY_Q);
    return ! finished;
}

void GuiManager::display()
{
    CB_GUIMANAGER_DEPIMPL();

    if (hasRoot())
    {
        root().display();
    }

    // Reset 4x rendering. Only needed for first frame after a mode change
    GuiDisplayable::useFourTimesRender() = false;
}

//////////////////////////////////////////////////////////////////////

// static
Gui::ButtonState buttonState(DevButtonEvent::Action a)
{
    return (a == DevButtonEvent::PRESS) ? Gui::PRESSED : Gui::RELEASED;
}

// static
Gui::ButtonState buttonState(bool isPressed)
{
    return isPressed ? Gui::PRESSED : Gui::RELEASED;
}

//////////////////////////////////////////////////////////////////////

GuiEventBase::ModifierKeys pollModifierKeys()
{
    GuiEventBase::ModifierKeys result = 0;

    if (DevKeyboard::instance().shiftPressed())
        result |= GuiEventBase::SHIFT;
    if (DevKeyboard::instance().ctrlPressed())
        result |= GuiEventBase::CTRL;
    if (DevKeyboard::instance().altPressed())
        result |= GuiEventBase::ALT;

    return result;
}

GuiMouseEvent pollMouse()
{
    Gui::ButtonState left = buttonState(DevMouse::instance().leftButton());
    Gui::ButtonState right = buttonState(DevMouse::instance().rightButton());

    ASSERT(left != DevMouse::NO_CHANGE, "logic error");
    ASSERT(right != DevMouse::NO_CHANGE, "logic error");

    Gui::Coord c(DevMouse::instance().position().first, DevMouse::instance().position().second);
    return GuiMouseEvent(c, left, right, pollModifierKeys());
}

//////////////////////////////////////////////////////////////////////

Gui::Vec unaryMinus(const Gui::Coord& c)
{
    return Gui::Vec(-c.x(), -c.y());
}

void GuiManager::changeRoot(GuiRoot* pNewRoot)
{
    CB_GUIMANAGER_DEPIMPL();

    pMouseFocus_ = nullptr;

    pRoot_ = pNewRoot;

    GuiDisplayable::useFourTimesRender() = true;
}

void GuiManager::updateMouseFocus()
{
    CB_GUIMANAGER_DEPIMPL();

    GuiMouseEvent me = pollMouse();

    GuiDisplayable* pFocus = nullptr;
    const Gui::Coord& p = me.coord();

    // Note : This probably isn't that safe if we end up with multiple devices.
    RenDisplay* pDisplay = RenDevice::current()->display();
    // If there is a mouse cursor on screen then find out what it's pointing at
    if (pDisplay->currentCursor() != nullptr)
        pFocus = pRoot_->innermostContainingCheckProcessesMouseEvents(p);

    ASSERT(implies(pFocus, pFocus->processesMouseEvents()), "logic error");

#ifndef PRODUCTION
    // ##NA include for useful debug info. Tells you what the mouse is pointing at
    if (getenv("cb_mouseover") && pFocus)
        W4dManager::instance().sceneManager()->out()
            << "Mouse is over " << pFocus->description() << " : " << pFocus << " " << pFocus->relativeCoord()
            << " processesMouseEvents " << pFocus->processesMouseEvents() << std::endl;
// ##NA
#endif

    bool focusChanged = pFocus != pMouseFocus_;
    if (focusChanged)
    {
        if (pMouseFocus_ != nullptr)
        {
            GuiMouseEvent meTmp = me;
            meTmp.translate(unaryMinus(pMouseFocus_->absoluteCoord()));
            pMouseFocus_->doHandleMouseExitEvent(meTmp);
        }

        if (pFocus)
        {
            pMouseFocus_ = pFocus;
            me.translate(unaryMinus(pMouseFocus_->absoluteCoord()));
            pMouseFocus_->doHandleMouseEnterEvent(me);
        }
        else
        {
            pMouseFocus_ = nullptr;
        }
    }
    else
    {
        if (pMouseFocus_ != nullptr)
        {
            me.translate(unaryMinus(pMouseFocus_->absoluteCoord()));
            pMouseFocus_->doHandleContainsMouseEvent(me);
        }
    }
}

void GuiManager::processMouseEvent(const GuiMouseEvent& me)
{
    CB_GUIMANAGER_DEPIMPL();

    GuiMouse::instance().update(me.coord());

    GuiMouseEvent mrel = me;
    mrel.translate(unaryMinus(root().absoluteCoord()));

    // If we have a right click, give the current root the opportunity to handle it.
    // Many applications will treat right clicks irrespective of location.
    bool handledRightClick
        = (me.rightButton() != Gui::NO_CHANGE) && hasRoot() && root().doHandleRightClickEvent(mrel);

    bool isMouseWheelEvent = (me.scrollDirection() != Gui::ScrollState::NO_SCROLL);

    if (pMouseFocus_ != nullptr)
    {
        if (isMouseWheelEvent)
        {
            mrel = me;
            mrel.translate(unaryMinus(pMouseFocus_->absoluteCoord()));
            pMouseFocus_->doHandleMouseScrollEvent(&mrel);
            if (!mrel.isAccepted())
            {
                GuiDisplayable* pParent = pMouseFocus_->parent();
                if (pParent)
                {
                    mrel.translate(unaryMinus(pParent->absoluteCoord()));
                    pParent->doHandleMouseScrollEvent(&mrel);
                }
            }
        }
        else if (! handledRightClick)
        {
            mrel = me;
            mrel.translate(unaryMinus(pMouseFocus_->absoluteCoord()));
            pMouseFocus_->doHandleMouseClickEvent(mrel);
        }
    }
}

//////////////////////////////////////////////////////////////////////

GuiEventBase::ModifierKeys modifierKeys(const DevButtonEvent& be)
{
    GuiEventBase::ModifierKeys result = 0;
    if (be.wasShiftPressed())
        result |= GuiEventBase::SHIFT;
    if (be.wasCtrlPressed())
        result |= GuiEventBase::CTRL;
    if (be.wasAltPressed())
        result |= GuiEventBase::ALT;

    return result;
}

//////////////////////////////////////////////////////////////////////

void GuiManager::processEvents()
{
    while (! DevEventQueue::instance().isEmpty())
    {
        DevButtonEvent be = DevEventQueue::instance().oldestEvent();
        GuiEventBase::ModifierKeys mKeys = modifierKeys(be);

        if (be.scanCode() == DevKey::MOUSE_LEFT)
        {
            GuiMouseEvent e(be.cursorCoords(), buttonState(be.action()), Gui::NO_CHANGE, mKeys);
            processMouseEvent(e);
        }
        else if (be.scanCode() == DevKey::MOUSE_RIGHT)
        {
            GuiMouseEvent e(be.cursorCoords(), Gui::NO_CHANGE, buttonState(be.action()), mKeys);
            processMouseEvent(e);
        }
        else if (be.scanCode() == DevKey::MOUSE_MIDDLE)
        {
            GuiMouseEvent e(be.cursorCoords(), Gui::NO_CHANGE, Gui::NO_CHANGE, getScrollDirection(be.action()), mKeys);
            processMouseEvent(e);
        }
        else if (be.isKeyEvent()) // keyboard event
        {
            if (keyboardFocusExists())
            {
                GuiKeyEvent e(be.scanCode(), buttonState(be.action()), mKeys, be);
                if (! keyboardFocus().doHandleKeyEvent(e))
                {
                    // If the keyboard focus doesn't want the event then maybe the char focus will
                    if (charFocusExists() && &charFocus() != &keyboardFocus())
                    {
                        charFocus().doHandleKeyEvent(e);
                    }
                }
            }
        }
        else if (be.isCharEvent()) // character event
        {
            if (charFocusExists())
            {
                charFocus().doHandleCharEvent(be);
            }
        }
    }
}

// static
inline Gui::ScrollState GuiManager::getScrollDirection(DevButtonEvent::Action act)
{
    switch (act)
    {
        case DevButtonEvent::SCROLL_UP:
            return Gui::ScrollState::SCROLL_UP;

        case DevButtonEvent::SCROLL_DOWN:
            return Gui::ScrollState::SCROLL_DOWN;

        case DevButtonEvent::PRESS:
        case DevButtonEvent::RELEASE:
        default:
            return Gui::ScrollState::NO_SCROLL;
    }
}

//////////////////////////////////////////////////////////////////////

const GuiColourScheme& GuiManager::colourScheme() const
{
    CB_GUIMANAGER_DEPIMPL();

    return colourScheme_;
}

//////////////////////////////////////////////////////////////////////

GuiColourScheme::GuiColourScheme()
    : mainBack_(Gui::LIGHTGREY())
    , mainInk_(Gui::BLACK())
    , borderBack_(Gui::LIGHTGREY())
    , borderLo_(Gui::DARKGREY())
    , borderHi_(Gui::WHITE())
    , headingBack_(Gui::BLUE())
    , headingInk_(Gui::WHITE())
    , buttonBack_(Gui::LIGHTGREY())
    , buttonInk_(Gui::BLACK())
    , buttonLo_(Gui::DARKGREY())
    , buttonHi_(Gui::WHITE())
    , buttonDefault_(Gui::BLACK())
{
    // Intentionally Empty
}

//////////////////////////////////////////////////////////////////////

const GuiColourScheme& GuiColourScheme::defaultScheme()
{
    static GuiColourScheme default_;
    return default_;
}

//////////////////////////////////////////////////////////////////////

const Gui::Colour& GuiColourScheme::mainBack() const
{
    return mainBack_;
}
const Gui::Colour& GuiColourScheme::mainInk() const
{
    return mainInk_;
}
const Gui::Colour& GuiColourScheme::borderBack() const
{
    return borderBack_;
}
const Gui::Colour& GuiColourScheme::borderLo() const
{
    return borderLo_;
}
const Gui::Colour& GuiColourScheme::borderHi() const
{
    return borderHi_;
}
const Gui::Colour& GuiColourScheme::headingBack() const
{
    return headingBack_;
}
const Gui::Colour& GuiColourScheme::headingInk() const
{
    return headingInk_;
}
const Gui::Colour& GuiColourScheme::buttonBack() const
{
    return buttonBack_;
}
const Gui::Colour& GuiColourScheme::buttonInk() const
{
    return buttonInk_;
}
const Gui::Colour& GuiColourScheme::buttonLo() const
{
    return buttonLo_;
}
const Gui::Colour& GuiColourScheme::buttonHi() const
{
    return buttonHi_;
}
const Gui::Colour& GuiColourScheme::buttonDefault() const
{
    return buttonDefault_;
}

//////////////////////////////////////////////////////////////////////

void GuiManager::isBeingDeleted(GuiDisplayable* pDisplayable)
{
    CB_GUIMANAGER_DEPIMPL();

    // Cancel any cached pointers
    if (pDisplayable == pMouseFocus_)
        pMouseFocus_ = nullptr;

    if (pDisplayable == pKeyboardFocus_)
        pKeyboardFocus_ = nullptr;

    if (pDisplayable == pCharacterFocus_)
        pCharacterFocus_ = nullptr;
}

void GuiManager::isBeingCreated(GuiDisplayable* /*pDisplayable*/)
{
}

//////////////////////////////////////////////////////////////////////

bool GuiManager::charFocusExists() const
{
    CB_GUIMANAGER_DEPIMPL();

    return pCharacterFocus_ != nullptr;
}

GuiDisplayable& GuiManager::charFocus()
{
    PRE(charFocusExists());

    CB_GUIMANAGER_DEPIMPL();

    return *pCharacterFocus_;
}

void GuiManager::charFocus(GuiDisplayable* pNewCharFocus)
{
    PRE(pNewCharFocus != nullptr);

    CB_GUIMANAGER_DEPIMPL();

    pCharacterFocus_ = pNewCharFocus;

    POST(charFocusExists());
}

void GuiManager::removeCharFocus()
{
    CB_GUIMANAGER_DEPIMPL();

    pCharacterFocus_ = nullptr;

    POST(! charFocusExists());
}

//////////////////////////////////////////////////////////////////////

/* End MANAGER.CPP **************************************************/
