#include "IConsole.hpp"

#include "utility/String.hpp"

namespace System
{

IConsole::CompletionProvider IConsole::commandNameCompleter() const
{
    return [this](
               const CommandMetadata&,
               std::size_t,
               std::string_view partialValue,
               const std::vector<std::string>&) -> std::vector<std::string>
    {
        std::vector<std::string> matches;
        for (const CommandMetadata& cmd : commands())
        {
            if (partialValue.empty() || Utils::startsWith(cmd.name, partialValue))
            {
                matches.push_back(cmd.name);
            }
        }
        return matches;
    };
}

} // namespace System
