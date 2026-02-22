#pragma once

namespace System
{

class IConsole;

// Registers built-in console commands: help, clear, echo.
void registerConsoleBuiltins(IConsole& console);

} // namespace System
