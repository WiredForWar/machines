#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Utils
{

template <std::forward_iterator Iterator>
std::string join(Iterator beginIt, Iterator endIt, std::string_view sep);

template <std::forward_iterator Iterator>
std::string join(Iterator beginIt, std::size_t num, std::string_view sep)
{
    return join(beginIt, std::next(beginIt, num), sep);
}

template <typename SeqContainer>
std::string join(const SeqContainer& container, std::string_view sep)
{
    return join(container.begin(), container.end(), sep);
}

std::vector<std::string_view> split(std::string_view path, char delimiter);

void replaceAll(std::string *str, std::string_view before, std::string_view after);

void toLowerInPlace(std::string *str);
std::string trimWhitespace(std::string_view text);

inline bool startsWith(std::string_view text, std::string_view prefix)
{
    return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

inline bool caseInsensitiveEqual(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    }
    return true;
}

} // namespace Utils
