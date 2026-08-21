#pragma once

#include "render/render.hpp"
#include "system/PathName.hpp"

#include <string>

class RenSurface;

namespace Gui
{

// Screenshots are kept together in one directory rather than dropped beside the
// executable, so that a session's worth of them is easy to find and to sweep up.
// The directory is made on the way if it is not there yet, and an unset path
// back means there is nowhere to write and nothing should be.

// The path this file name gets.
SysPathName screenshotPath(const std::string& fileName);

// The path of the first <prefix>NNNN.png that nothing occupies yet, counting up
// from 0000. Unset if every number is taken.
SysPathName nextScreenshotPath(const std::string& prefix);

// Writes the given area of the surface to the path, or the whole of it when the
// area is empty, and says whether the file ended up holding it. Where it went is
// logged either way, an unset path included: a screenshot that quietly did not
// happen is worse than one that says so.
bool saveScreenshot(const RenSurface& surface, const SysPathName& path, const Ren::Rect& area = Ren::Rect());

} // namespace Gui
