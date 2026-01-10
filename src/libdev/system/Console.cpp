#include "Console.hpp"

#include "utility/String.hpp"

namespace System
{

bool Console::registerCommand(const CommandMetadata& metadata, CommandHandler handler)
{
    if (metadata.name.empty() || !handler)
    {
        return false;
    }

    const std::string trimmed = Utils::trimWhitespace(metadata.name);
    if (trimmed.empty())
    {
        return false;
    }

    if (commands_.contains(trimmed))
    {
        return false;
    }

    CommandDefinition definition{};
    definition.metadata = metadata;
    definition.metadata.name = trimmed;
    definition.handler = std::move(handler);

    commands_.emplace(definition.metadata.name, std::move(definition));
    return true;
}

bool Console::submit(std::string_view line)
{
    const std::string trimmed = Utils::trimWhitespace(line);
    if (trimmed.empty())
    {
        return false;
    }

    const std::vector<std::string> tokens = {trimmed};
    if (tokens.empty())
    {
        return false;
    }

    const std::string commandName = tokens.front();
    const CommandMap::iterator commandIterator = commands_.find(commandName);
    if (commandIterator == commands_.end())
    {
        return false;
    }

    CommandRequest request{};
    request.name = commandName;
    request.rawLine = std::string(trimmed);
    commandIterator->second.handler(request);
    return true;
}

} // namespace System
