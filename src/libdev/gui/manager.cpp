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
    Device::KeyCode::UP_ARROW_PAD,
    Device::KeyCode::DOWN_ARROW_PAD,
    Device::KeyCode::LEFT_ARROW_PAD,
    Device::KeyCode::RIGHT_ARROW_PAD,
    Device::KeyCode::ENTER_PAD,
    Device::KeyCode::INSERT_PAD,
    Device::KeyCode::DELETE_PAD,
    Device::KeyCode::HOME_PAD,
    Device::KeyCode::END_PAD,
    Device::KeyCode::PAGE_UP_PAD,
    Device::KeyCode::PAGE_DOWN_PAD,
    Device::KeyCode::FIVE_PAD,
    Device::KeyCode::ASTERISK_PAD,
    Device::KeyCode::FORWARD_SLASH_PAD,
    Device::KeyCode::PLUS_PAD,
    Device::KeyCode::MINUS_PAD,
    Device::KeyCode::PAD_0,
    Device::KeyCode::PAD_1,
    Device::KeyCode::PAD_2,
    Device::KeyCode::PAD_3,
    Device::KeyCode::PAD_4,
    Device::KeyCode::PAD_5,
    Device::KeyCode::PAD_6,
    Device::KeyCode::PAD_7,
    Device::KeyCode::PAD_8,
    Device::KeyCode::PAD_9,
    Device::KeyCode::SCROLL_LOCK,
    Device::KeyCode::BREAK,
    Device::KeyCode::RIGHT_CONTROL,
    Device::KeyCode::LEFT_CONTROL,
    Device::KeyCode::UP_ARROW,
    Device::KeyCode::DOWN_ARROW,
    Device::KeyCode::LEFT_ARROW,
    Device::KeyCode::RIGHT_ARROW,
    Device::KeyCode::INSERT,
    Device::KeyCode::KEY_DELETE,
    Device::KeyCode::HOME,
    Device::KeyCode::END,
    Device::KeyCode::PAGE_UP,
    Device::KeyCode::PAGE_DOWN,
    Device::KeyCode::ESCAPE,
    Device::KeyCode::ENTER,
    Device::KeyCode::SPACE,
    Device::KeyCode::BACK_SPACE,
    Device::KeyCode::TAB,
    Device::KeyCode::GRAVE,
    Device::KeyCode::LEFT_SHIFT,
    Device::KeyCode::RIGHT_SHIFT,
    Device::KeyCode::CAPS_LOCK,
    Device::KeyCode::NUM_LOCK,
    Device::KeyCode::F1,
    Device::KeyCode::F2,
    Device::KeyCode::F3,
    Device::KeyCode::F4,
    Device::KeyCode::F5,
    Device::KeyCode::F6,
    Device::KeyCode::F7,
    Device::KeyCode::F8,
    Device::KeyCode::F9,
    Device::KeyCode::F10,
    Device::KeyCode::F11,
    Device::KeyCode::F12,
    Device::KeyCode::KEY_A,
    Device::KeyCode::KEY_B,
    Device::KeyCode::KEY_C,
    Device::KeyCode::KEY_D,
    Device::KeyCode::KEY_E,
    Device::KeyCode::KEY_F,
    Device::KeyCode::KEY_G,
    Device::KeyCode::KEY_H,
    Device::KeyCode::KEY_I,
    Device::KeyCode::KEY_J,
    Device::KeyCode::KEY_K,
    Device::KeyCode::KEY_L,
    Device::KeyCode::KEY_M,
    Device::KeyCode::KEY_N,
    Device::KeyCode::KEY_O,
    Device::KeyCode::KEY_P,
    Device::KeyCode::KEY_Q,
    Device::KeyCode::KEY_R,
    Device::KeyCode::KEY_S,
    Device::KeyCode::KEY_T,
    Device::KeyCode::KEY_U,
    Device::KeyCode::KEY_V,
    Device::KeyCode::KEY_W,
    Device::KeyCode::KEY_X,
    Device::KeyCode::KEY_Y,
    Device::KeyCode::KEY_Z,
    Device::KeyCode::KEY_0,
    Device::KeyCode::KEY_1,
    Device::KeyCode::KEY_2,
    Device::KeyCode::KEY_3,
    Device::KeyCode::KEY_4,
    Device::KeyCode::KEY_5,
    Device::KeyCode::KEY_6,
    Device::KeyCode::KEY_7,
    Device::KeyCode::KEY_8,
    Device::KeyCode::KEY_9,
    Device::KeyCode::MOUSE_LEFT,
    Device::KeyCode::MOUSE_RIGHT,
    Device::KeyCode::MOUSE_MIDDLE,
    Device::KeyCode::MOUSE_EXTRA1,
    Device::KeyCode::MOUSE_EXTRA2,
    Device::KeyCode::MOUSE_EXTRA3,
    Device::KeyCode::MOUSE_EXTRA4,
    Device::KeyCode::MOUSE_EXTRA5,
    Device::KeyCode::MOUSE_EXTRA6,
    Device::KeyCode::MOUSE_EXTRA7,
    Device::KeyCode::MOUSE_EXTRA8,
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
    DevEventQueue::instance().queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);
    DevEventQueue::instance().queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN);
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

    bool finished = DevKeyboard::instance().keyCode(Device::KeyCode::KEY_Q);
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

        if (be.scanCode() == Device::KeyCode::MOUSE_LEFT)
        {
            GuiMouseEvent e(be.cursorCoords(), buttonState(be.action()), Gui::NO_CHANGE, mKeys);
            processMouseEvent(e);
        }
        else if (be.scanCode() == Device::KeyCode::MOUSE_RIGHT)
        {
            GuiMouseEvent e(be.cursorCoords(), Gui::NO_CHANGE, buttonState(be.action()), mKeys);
            processMouseEvent(e);
        }
        else if (be.scanCode() == Device::KeyCode::MOUSE_MIDDLE)
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
