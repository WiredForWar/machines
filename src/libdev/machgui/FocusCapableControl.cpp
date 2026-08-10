/*
 * F O C U S C T L . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/FocusCapableControl.hpp"
#include "machgui/StartupScreens.hpp"

MachGuiFocusCapableControl::MachGuiFocusCapableControl(MachGuiStartupScreens* pStartupScreens)
{
    pStartupScreens_ = pStartupScreens;
    hasFocus_ = false;

    // Add this to StartupScreens collection of focus capable controls
    pStartupScreens_->addFocusCapableControl(this);

    TEST_INVARIANT;
}

MachGuiFocusCapableControl::~MachGuiFocusCapableControl()
{
    TEST_INVARIANT;

    // Remove this to StartupScreens collection of focus capable controls
    pStartupScreens_->removeFocusCapableControl(this);
}

void MachGuiFocusCapableControl::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiFocusCapableControl& t)
{
    o << "MachGuiFocusCapableControl " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiFocusCapableControl " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
bool MachGuiFocusCapableControl::isFocusEnabled() const
{
    return !msgBoxIsDisplayed_;
}

// virtual
bool MachGuiFocusCapableControl::executeControl()
{
    PRE(isFocusControl());
    PRE(isFocusEnabled());

    return false;
}

// virtual
bool MachGuiFocusCapableControl::isFocusControl() const
{
    return hasFocus_ && ! msgBoxIsDisplayed_;
}

// virtual
void MachGuiFocusCapableControl::hasFocus(bool newValue)
{
    hasFocus_ = newValue;
}

// virtual
bool MachGuiFocusCapableControl::doHandleNavigationKey(
    MachGuiFocusCapableControl::NavKey navKey,
    MachGuiFocusCapableControl** ppNavFocusControl)
{
    switch (navKey)
    {
        case LEFT_ARROW:
            *ppNavFocusControl = pLeftNavControl_;
            break;
        case RIGHT_ARROW:
            *ppNavFocusControl = pRightNavControl_;
            break;
        case UP_ARROW:
            *ppNavFocusControl = pUpNavControl_;
            break;
        case DOWN_ARROW:
            *ppNavFocusControl = pDownNavControl_;
            break;
        case TAB_BACKWARD:
            *ppNavFocusControl = pTabBackwardNavControl_;
            break;
        case TAB_FOWARD:
            *ppNavFocusControl = pTabFowardNavControl_;
            break;
    }

    return ((*ppNavFocusControl != nullptr) && (*ppNavFocusControl)->isFocusEnabled());
}

void MachGuiFocusCapableControl::setLeftNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pLeftNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setRightNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pRightNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setUpNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pUpNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setDownNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pDownNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setTabFowardNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pTabFowardNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setTabBackwardDownNavControl(MachGuiFocusCapableControl* pNewValue)
{
    pTabBackwardNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::msgBoxIsDisplayed(bool newValue)
{
    msgBoxIsDisplayed_ = newValue;
}

void MachGuiFocusCapableControl::escapeControl(bool newValue)
{
    escapeControl_ = newValue;
}

bool MachGuiFocusCapableControl::isEscapeControl() const
{
    return escapeControl_;
}

void MachGuiFocusCapableControl::defaultControl(bool newValue)
{
    defaultControl_ = newValue;
}

bool MachGuiFocusCapableControl::isDefaultControl() const
{
    return defaultControl_;
}

// virtual
bool MachGuiFocusCapableControl::hasFocusSet() const
{
    return hasFocus_;
}

/* End FOCUSCTL.CPP *************************************************/
