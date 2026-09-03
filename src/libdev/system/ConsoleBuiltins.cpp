#include "system/ConsoleBuiltins.hpp"

#include "system/IConsole.hpp"

#include <sstream>

namespace System
{

static void helpCommand(const IConsole::CommandRequest& request, IConsole& console)
{
    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        const std::vector<IConsole::CommandMetadata> allCommands = console.commands();
        for (const IConsole::CommandMetadata& cmd : allCommands)
        {
            std::string line = cmd.name;
            if (!cmd.description.empty())
            {
                line += " - ";
                line += cmd.description;
            }
            console.writeLine(line);
        }
        return;
    }

    const std::string& commandName = std::get<std::string>(request.arguments[0].value);
    const std::vector<IConsole::CommandMetadata> allCommands = console.commands();
    for (const IConsole::CommandMetadata& cmd : allCommands)
    {
        if (cmd.name == commandName)
        {
            std::string line = cmd.name;
            if (!cmd.description.empty())
            {
                line += " - ";
                line += cmd.description;
            }
            console.writeLine(line);

            if (!cmd.arguments.empty())
            {
                console.writeLine("Arguments:");
                for (const IConsole::ArgumentSpec& arg : cmd.arguments)
                {
                    std::ostringstream oss;
                    oss << "  " << arg.name;
                    if (arg.optional)
                        oss << " (optional)";
                    if (!arg.description.empty())
                        oss << " - " << arg.description;
                    console.writeLine(oss.str());
                }
            }
            return;
        }
    }

    console.writeLine("Unknown command: " + commandName);
}

static void clearCommand(const IConsole::CommandRequest&, IConsole& console)
{
    console.clearOutput();
}

static void echoCommand(const IConsole::CommandRequest& request, IConsole& console)
{
    if (!request.arguments.empty() && request.arguments[0].provided)
    {
        console.writeLine(std::get<std::string>(request.arguments[0].value));
    }
}

static void blockModeCommand(const IConsole::CommandRequest& request, IConsole& console)
{
    if (request.arguments.empty() || !request.arguments[0].provided)
    {
        console.writeLine(std::string("Blocking mode is ") + (console.blockModeEnabled() ? "on" : "off") + ".");
        return;
    }

    const bool enabled = std::get<bool>(request.arguments[0].value);
    console.setBlockModeEnabled(enabled);
    console.writeLine(std::string("Blocking mode turned ") + (enabled ? "on" : "off") + ".");
}

void registerConsoleBuiltins(IConsole& console)
{
    console.registerCommand(
        {
            .name = "help",
            .description = "List all commands or show help for a specific command.",
            .arguments = { { .name = "command", .type = IConsole::ArgumentType::String, .optional = true, .description = "Command name to get help for." } },
        },
        helpCommand,
        console.commandNameCompleter());

    console.registerCommand(
        {
            .name = "clear",
            .description = "Clear the console output.",
        },
        clearCommand);

    console.registerCommand(
        {
            .name = "echo",
            .description = "Print text to the console.",
            .arguments = { { .name = "text", .type = IConsole::ArgumentType::String, .optional = false, .description = "Text to print." } },
        },
        echoCommand);

    console.registerCommand(
        {
            .name = "block_mode",
            .description = "Get/set whether a command that finishes later holds back the input.",
            .arguments = { { .name = "state", .type = IConsole::ArgumentType::Boolean, .optional = true, .description = "on or off. Omit to print current state." } },
        },
        blockModeCommand);
}

} // namespace System
