/*
 * E R R O R H N D . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "planeted/errorhnd.hpp"
#include "render/display.hpp"

// static
ErrorHandler& ErrorHandler::instance()
{
    static ErrorHandler instance_;
    return instance_;
}

ErrorHandler::ErrorHandler()
    : pDisplay_(nullptr)
{
#ifndef NDEBUG
    BaseAssertion::set_assertion_handler(handleAssertionError);
#endif

    TEST_INVARIANT;
}

ErrorHandler::~ErrorHandler()
{
    TEST_INVARIANT;
}

void ErrorHandler::pDisplay(RenDisplay* pDisplay)
{
    pDisplay_ = pDisplay;
}

#ifndef NDEBUG
// static
BaseAssertion::AssertionAction ErrorHandler::handleAssertionError(const BaseAssertion::AssertionInfo&)
{
    //  Get rid of the display so that we get the screen back
    if (ErrorHandler::instance().pDisplay_)
        delete ErrorHandler::instance().pDisplay_;

    ErrorHandler::instance().pDisplay_ = nullptr;

    return BaseAssertion::ASSERT_FAIL;
}
#endif

void ErrorHandler::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

/* End ERRORHND.CPP *************************************************/
