#pragma once

#include "system/IConsole.hpp"

#include <unordered_map>

namespace System
{

class Console : public IConsole
{
public:
    bool registerCommand(const CommandMetadata& metadata, CommandHandler handler) override;

    bool submit(std::string_view line) override;

private:
    struct CommandDefinition
    {
        CommandMetadata metadata{};
        CommandHandler handler{};
    };

    using CommandMap = std::unordered_map<std::string, CommandDefinition>;

    CommandMap commands_{};
};

} // namespace System
