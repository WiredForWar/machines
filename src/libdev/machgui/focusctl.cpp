/*
 * F O C U S C T L . C P P
 * (c) Charybdis Limited, 1999. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/focusctl.hpp"
#include "machgui/startup.hpp"

class MachGuiFocusCapableControlImpl
{
private:
    MachGuiFocusCapableControlImpl()
        : hasFocus_(false)
        , pStartupScreens_(nullptr)
        , pLeftNavControl_(nullptr)
        , pRightNavControl_(nullptr)
        , pUpNavControl_(nullptr)
        , pDownNavControl_(nullptr)
        , pTabFowardNavControl_(nullptr)
        , pTabBackwardNavControl_(nullptr)
        , msgBoxIsDisplayed_(false)
        , escapeControl_(false)
        , defaultControl_(false)
    {
    }

    // Data members
    bool hasFocus_;
    MachGuiStartupScreens* pStartupScreens_;
    MachGuiFocusCapableControl* pLeftNavControl_;
    MachGuiFocusCapableControl* pRightNavControl_;
    MachGuiFocusCapableControl* pUpNavControl_;
    MachGuiFocusCapableControl* pDownNavControl_;
    MachGuiFocusCapableControl* pTabFowardNavControl_;
    MachGuiFocusCapableControl* pTabBackwardNavControl_;
    bool msgBoxIsDisplayed_;
    bool escapeControl_;
    bool defaultControl_;

    friend class MachGuiFocusCapableControl;
};

MachGuiFocusCapableControl::MachGuiFocusCapableControl(MachGuiStartupScreens* pStartupScreens)
{
    pImpl_ = new MachGuiFocusCapableControlImpl();

    CB_DEPIMPL(bool, hasFocus_);
    CB_DEPIMPL(MachGuiStartupScreens*, pStartupScreens_);

    pStartupScreens_ = pStartupScreens;
    hasFocus_ = false;

    // Add this to StartupScreens collection of focus capable controls
    pStartupScreens_->addFocusCapableControl(this);

    TEST_INVARIANT;
}

MachGuiFocusCapableControl::~MachGuiFocusCapableControl()
{
    TEST_INVARIANT;

    CB_DEPIMPL(MachGuiStartupScreens*, pStartupScreens_);

    // Remove this to StartupScreens collection of focus capable controls
    pStartupScreens_->removeFocusCapableControl(this);

    delete pImpl_;
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
    CB_DEPIMPL(bool, msgBoxIsDisplayed_);

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
    CB_DEPIMPL(bool, hasFocus_);
    CB_DEPIMPL(bool, msgBoxIsDisplayed_);

    return hasFocus_ && ! msgBoxIsDisplayed_;
}

// virtual
void MachGuiFocusCapableControl::hasFocus(bool newValue)
{
    CB_DEPIMPL(bool, hasFocus_);

    hasFocus_ = newValue;
}

// virtual
bool MachGuiFocusCapableControl::doHandleNavigationKey(
    MachGuiFocusCapableControl::NavKey navKey,
    MachGuiFocusCapableControl** ppNavFocusControl)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pLeftNavControl_);
    CB_DEPIMPL(MachGuiFocusCapableControl*, pRightNavControl_);
    CB_DEPIMPL(MachGuiFocusCapableControl*, pUpNavControl_);
    CB_DEPIMPL(MachGuiFocusCapableControl*, pDownNavControl_);
    CB_DEPIMPL(MachGuiFocusCapableControl*, pTabFowardNavControl_);
    CB_DEPIMPL(MachGuiFocusCapableControl*, pTabBackwardNavControl_);

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
    CB_DEPIMPL(MachGuiFocusCapableControl*, pLeftNavControl_);

    pLeftNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setRightNavControl(MachGuiFocusCapableControl* pNewValue)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pRightNavControl_);

    pRightNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setUpNavControl(MachGuiFocusCapableControl* pNewValue)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pUpNavControl_);

    pUpNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setDownNavControl(MachGuiFocusCapableControl* pNewValue)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pDownNavControl_);

    pDownNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setTabFowardNavControl(MachGuiFocusCapableControl* pNewValue)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pTabFowardNavControl_);

    pTabFowardNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::setTabBackwardDownNavControl(MachGuiFocusCapableControl* pNewValue)
{
    CB_DEPIMPL(MachGuiFocusCapableControl*, pTabBackwardNavControl_);

    pTabBackwardNavControl_ = pNewValue;
}

void MachGuiFocusCapableControl::msgBoxIsDisplayed(bool newValue)
{
    CB_DEPIMPL(bool, msgBoxIsDisplayed_);

    msgBoxIsDisplayed_ = newValue;
}

void MachGuiFocusCapableControl::escapeControl(bool newValue)
{
    CB_DEPIMPL(bool, escapeControl_);

    escapeControl_ = newValue;
}

bool MachGuiFocusCapableControl::isEscapeControl() const
{
    CB_DEPIMPL(bool, escapeControl_);

    return escapeControl_;
}

void MachGuiFocusCapableControl::defaultControl(bool newValue)
{
    CB_DEPIMPL(bool, defaultControl_);

    defaultControl_ = newValue;
}

bool MachGuiFocusCapableControl::isDefaultControl() const
{
    CB_DEPIMPL(bool, defaultControl_);

    return defaultControl_;
}

// virtual
bool MachGuiFocusCapableControl::hasFocusSet() const
{
    CB_DEPIMPL(bool, hasFocus_);

    return hasFocus_;
}

/* End FOCUSCTL.CPP *************************************************/
