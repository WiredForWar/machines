#pragma once

#include "render/render.hpp"
#include "system/PathName.hpp"

#include <optional>
#include <string>

class RenSurface;

namespace Gui
{

// Screenshots are kept together in one directory rather than dropped beside the
// executable, so that a session's worth of them is easy to find and to sweep up.
// The directory is made when the first one is written, not when its path is
// worked out, so asking where a shot would go costs nothing and changes nothing.

// Why this is not a name a screenshot may have, or nothing if it is fine. A name
// is a bare file name of letters, digits, underscores and dots that starts with
// a letter or a digit, so it can never reach outside the one directory they all
// go in.
std::optional<std::string> screenshotNameComplaint(const std::string& fileName);

// The path this name gets, with .png put on the end unless it is already there.
// A name worth passing here is one screenshotNameComplaint had nothing to say
// about.
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
