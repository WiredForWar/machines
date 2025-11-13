#include "utility/string.hpp"

#include <vector>

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

namespace
{

using StrIt = std::vector<std::string>::iterator;
using StrConstIt = std::vector<std::string>::const_iterator;

} // namespace

template std::string join<>(StrIt, StrIt, std::string_view);
template std::string join<>(StrConstIt, StrConstIt, std::string_view);

} // namespace Utils
