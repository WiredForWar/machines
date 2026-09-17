#pragma once

#include <optional>
#include <string>
#include <string_view>

//  Where the log of one run goes when the caller names it.
//
//  Every log belongs under logs/, so the name is read as a path relative to
//  that and nowhere else. A name that could point outside is refused rather
//  than quietly redirected.
//
//  Returns the path to open, or nothing when the name points nowhere under
//  logs/, setting whyNot to the reason when one was asked for. Any missing
//  directories along the way are made by the sink when it opens the file.
std::optional<std::string> afxLogFilePath(std::string_view given, std::string* whyNot = nullptr);
