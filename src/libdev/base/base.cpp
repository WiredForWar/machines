/*
 * B A S E . C P P
 * (c) Charybdis Limited, 1996. All Rights Reserved.
 */

#include "base/base.hpp"

#include <cstdlib>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

//////////////////////////////////////////////////////////////////////

namespace Base
{

void abortWithoutADialog()
{
#ifdef _MSC_VER
    // Without this the debug runtime puts up an abort dialog, and the reporting
    // one behind it, and waits.
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    // The library's own assertions report to stderr rather than a dialog too.
    for (int report : { _CRT_WARN, _CRT_ERROR, _CRT_ASSERT })
    {
        _CrtSetReportMode(report, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(report, _CRTDBG_FILE_STDERR);
    }
#endif
}

} // namespace Base

//////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

//////////////////////////////////////////////////////////////////////

BaseAssertion::AssertionLock& BaseAssertion::AssertionLock::instance()
{
    static thread_local AssertionLock lock;
    return lock;
}

//////////////////////////////////////////////////////////////////////

void BaseAssertion::AssertionLock::lock()
{
    if (isLocked_)
        BaseAssertion::preconditionFail("not isLocked()", __FILE__, _STR(__LINE__));

    isLocked_ = true;
}

void BaseAssertion::AssertionLock::unlock()
{
    if (! isLocked_)
        BaseAssertion::preconditionFail("isLocked()", __FILE__, _STR(__LINE__));

    isLocked_ = false;
}

void BaseAssertion::AssertionLock::lockInvariant()
{
    if (invariantLocked_)
        BaseAssertion::preconditionFail("not invariantLocked()", __FILE__, _STR(__LINE__));

    invariantLocked_ = true;
}

void BaseAssertion::AssertionLock::unlockInvariant()
{
    if (! invariantLocked_)
        BaseAssertion::preconditionFail("invariantLocked()", __FILE__, _STR(__LINE__));

    invariantLocked_ = false;
}

bool BaseAssertion::AssertionLock::isLocked() const
{
    return isLocked_;
}

bool BaseAssertion::AssertionLock::invariantLocked() const
{
    return invariantLocked_;
}

BaseAssertion::AssertionLock::AssertionLock()
    : isLocked_(false)
    , invariantLocked_(false)
{
    /* Intentionally Empty  */
}

#endif

//////////////////////////////////////////////////////////////////////

/* End BASE.CPP *****************************************************/
