#pragma once

#include <string>

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

} // namespace Utils
