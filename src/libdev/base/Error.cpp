/*
 * E R R O R . C P P
 */

#include "base/Error.hpp"
#include "system/WindowsAPI.hpp"

namespace BaseErr {
void TerminateOnError(const char* pMsg)
{
    SysWindowsAPI::messageBoxError(pMsg, "Error");
    exit(1);
}
} // namespace BaseErr

/* End ERROR.CPP *************************************************/
