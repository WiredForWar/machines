#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace System
{

class IConsole
{
public:
    struct CommandMetadata
    {
        std::string name{};
        std::string description{};
    };

    struct CommandRequest
    {
        std::string name{};
        std::string rawLine{};
    };

    using CommandHandler = std::function<void(const CommandRequest&)>;

    virtual ~IConsole() = default;

    virtual bool registerCommand(const CommandMetadata& metadata, CommandHandler handler) = 0;

    virtual bool submit(std::string_view line) = 0;
};

} // namespace System
