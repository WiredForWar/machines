#include "afx/AfxLogFile.hpp"

#include <algorithm>

namespace
{

bool isSeparator(char c)
{
    return c == '/' || c == '\\';
}

std::optional<std::string> refuse(const char* reason, std::string* whyNot)
{
    if (whyNot)
        *whyNot = reason;

    return std::nullopt;
}

} // namespace

std::optional<std::string> afxLogFilePath(std::string_view given, std::string* whyNot)
{
    if (given.empty())
        return refuse("no name given", whyNot);

    if (isSeparator(given.front()))
        return refuse("an absolute path, and the log goes under logs/", whyNot);

    //  "c:name" is not absolute but is read against that drive's own current
    //  directory, which is not this one.
    if (given.size() > 1 && given[1] == ':')
        return refuse("a path against another drive, and the log goes under logs/", whyNot);

    //  One kind of separator to walk below, and a name written with backslashes
    //  works on the systems that do not treat those as separators at all.
    std::string path{given};
    std::replace(path.begin(), path.end(), '\\', '/');

    if (path.back() == '/')
        return refuse("a directory, not a file", whyNot);

    for (std::size_t start = 0; start <= path.size();)
    {
        const std::size_t end = std::min(path.find('/', start), path.size());
        if (std::string_view(path).substr(start, end - start) == "..")
            return refuse("a path that climbs out of logs/", whyNot);

        if (end == path.size())
            break;

        start = end + 1;
    }

    return "logs/" + path;
}
