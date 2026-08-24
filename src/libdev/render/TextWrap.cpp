#include "render/TextWrap.hpp"

#include "render/BmpFont.hpp"

namespace Ren
{

std::vector<std::string_view>
wrapText(std::string_view text, int maxWidth, const std::function<int(std::string_view)>& widthOf)
{
    std::vector<std::string_view> lines;

    std::size_t start = 0;
    while (start < text.size())
    {
        const std::string_view rest = text.substr(start);

        if (widthOf(rest) <= maxWidth)
        {
            lines.push_back(rest);
            break;
        }

        std::size_t fits = 0;
        while (fits < rest.size() && widthOf(rest.substr(0, fits + 1)) <= maxWidth)
            ++fits;

        // Nothing fits at all, which a width narrower than a single character
        // does. Take one anyway: a tall column of single characters is poor, but
        // it ends, where a line that keeps nothing never would.
        if (fits == 0)
            fits = 1;

        std::size_t keep = fits;
        std::size_t drop = 0;

        // The last space among the final TextWrapLookBack characters that fit.
        // The first character is not a break: it would put out an empty line and
        // swallow the space without making anything shorter.
        const std::size_t windowStart = fits > TextWrapLookBack ? fits - TextWrapLookBack : 0;
        for (std::size_t i = fits; i-- > windowStart;)
        {
            if (rest[i] == ' ' && i != 0)
            {
                keep = i;
                drop = 1;
                break;
            }
        }

        lines.push_back(rest.substr(0, keep));
        start += keep + drop;
    }

    // Empty text is a line of its own, so that a blank line of output stays one.
    if (lines.empty())
        lines.push_back(text);

    return lines;
}

std::vector<std::string_view> wrapText(std::string_view text, int maxWidth, const BmpFont& font)
{
    return wrapText(text, maxWidth, [&font](std::string_view part) { return font.horizontalAdvance(part); });
}

} // namespace Ren
