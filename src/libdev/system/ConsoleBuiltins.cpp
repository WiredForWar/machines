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

void registerConsoleBuiltins(IConsole& console)
{
    console.registerCommand(
        {
            "help",
            "List all commands or show help for a specific command.",
            { { "command", IConsole::ArgumentType::String, true, "Command name to get help for." } },
        },
        helpCommand);

    console.registerCommand(
        {
            "clear",
            "Clear the console output.",
        },
        clearCommand);

    console.registerCommand(
        {
            "echo",
            "Print text to the console.",
            { { "text", IConsole::ArgumentType::String, false, "Text to print." } },
        },
        echoCommand);
}

} // namespace System
