#pragma once

#include <functional>
#include <string_view>
#include <vector>

#include <cstddef>

namespace Ren
{

class BmpFont;

// How far back from a break a space is worth looking for, in characters. Past
// this the word is long enough that cutting it where it stops fitting reads
// better than the short line keeping it whole would leave behind.
inline constexpr std::size_t TextWrapLookBack = 16;

// The lines text takes to fit within maxWidth, measured by widthOf. A line breaks
// at the last space among the final TextWrapLookBack characters that fit, and
// after the last character that fits when there is no such space. The space
// broken at is dropped, and nothing is put in front of the lines that follow.
//
// Text that already fits comes back as a single line, and so does empty text, so
// the result is never empty and a line of output never disappears. The views
// point into text, which has to outlive them.
std::vector<std::string_view>
wrapText(std::string_view text, int maxWidth, const std::function<int(std::string_view)>& widthOf);

// The lines text takes to fit within maxWidth when drawn in font.
std::vector<std::string_view> wrapText(std::string_view text, int maxWidth, const BmpFont& font);

} // namespace Ren
