#pragma once

#include "system/PathName.hpp"

namespace MachGui
{

// Makes the directory saved games go in, if it is not there yet.
void prepareSaveGameDirectory();

// The path to save to next, which is the first numbered name the directory
// does not already hold.
SysPathName nextSaveGamePath();

} // namespace MachGui
