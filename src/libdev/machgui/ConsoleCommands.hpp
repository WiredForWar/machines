#pragma once

class MachGuiStartupScreens;

namespace System
{

class IConsole;

} // namespace System

namespace MachGui
{

// Registers game-specific console commands (camera, resources, spawning, etc.).
// Commands that need the in-game screen access it lazily via pStartupScreens->inGameScreen().
void registerConsoleCommands(System::IConsole& console, MachGuiStartupScreens* pStartupScreens);

} // namespace MachGui
