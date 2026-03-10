#include "utility/String.hpp"

#include <cctype>

namespace Utils
{

template<std::forward_iterator Iterator>
std::string join(Iterator beginIt, Iterator endIt, std::string_view sep)
{
    std::string result;
    if (beginIt != endIt)
    {
        std::size_t finalSize{};
        for (auto it = beginIt; it != endIt; ++it)
        {
            const std::string_view v = *it;
            finalSize += v.size();
        }
        finalSize += std::distance(beginIt, endIt) * sep.size();

        result.reserve(finalSize);
        result += *beginIt;

        for (auto it = std::next(beginIt); it != endIt; ++it)
        {
            result += sep;
            const std::string_view v = *it;
            result += v;
        }
    }

    return result;
}

std::vector<std::string_view> split(std::string_view path, char delimiter)
{
    std::vector<std::string_view> pathComponents;
    std::size_t pos = 0;
    std::size_t fromPos = 0;
    while ((pos = path.find(delimiter, fromPos)) != std::string::npos)
    {
        pathComponents.push_back(path.substr(fromPos, pos - fromPos));
        fromPos = pos + 1;
    }
    pathComponents.push_back(path.substr(fromPos));
    return pathComponents;
}

void replaceAll(std::string* str, std::string_view before, std::string_view after)
{
    std::size_t pos = str->find(before);
    while (pos != std::string::npos)
    {
        str->replace(pos, before.size(), after);
        // Get the next occurrence from the end of the replaced section
        pos = str->find(before, pos + after.size());
    }
}

void toLowerInPlace(std::string* str)
{
    if (str == nullptr || str->empty())
        return;

    for (char& ch : *str)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

std::string trimWhitespace(std::string_view text)
{
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
    {
        return {};
    }
    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(first, last - first + 1));
}

namespace
{

using StrIt = std::vector<std::string>::iterator;
using StrConstIt = std::vector<std::string>::const_iterator;

} // namespace

template std::string join<>(StrIt, StrIt, std::string_view);
template std::string join<>(StrConstIt, StrConstIt, std::string_view);

} // namespace Utils
