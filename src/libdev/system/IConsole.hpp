#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "utility/CallbackHandle.hpp"

namespace System
{

class IConsole
{
public:
    enum class ArgumentType
    {
        Boolean,
        Integer,
        Float,
        String,
        Identifier,
    };

    struct ArgumentSpec
    {
        std::string name{};
        ArgumentType type{};
        bool optional{};
        std::string description{};
    };

    struct CommandMetadata
    {
        std::string name{};
        std::string description{};
        std::vector<ArgumentSpec> arguments{};
        bool cheat{};
    };

    struct ArgumentValue
    {
        ArgumentType type{};
        bool provided{};
        std::variant<bool, std::int64_t, double, std::string> value{std::string()};
    };

    struct CommandRequest
    {
        std::string name{};
        std::vector<ArgumentValue> arguments{};
        std::string rawLine{};
    };

    struct OutputEvent
    {
        enum class Type
        {
            AppendLine,
            Clear,
        };

        Type type{};
        std::string_view text{};
    };

    struct CompletionResult
    {
        std::vector<std::string> candidates{};
        std::size_t replaceStart{};
        std::size_t replaceLength{};
        std::size_t totalCount{};
    };

    using CommandHandler = std::function<void(const CommandRequest&, IConsole&)>;
    using CompletionProvider = std::function<std::vector<std::string>(
        const CommandMetadata& metadata,
        std::size_t argIndex,
        std::string_view partialValue,
        const std::vector<std::string>& precedingArgs)>;
    using OutputListener = std::function<void(const OutputEvent&)>;

    virtual ~IConsole() = default;

    // Returns a CompletionProvider that completes registered command names by prefix.
    [[nodiscard]] CompletionProvider commandNameCompleter() const;

    virtual bool registerCommand(const CommandMetadata& metadata, CommandHandler handler) = 0;
    virtual bool
    registerCommand(const CommandMetadata& metadata, CommandHandler handler, CompletionProvider completer)
        = 0;
    virtual bool unregisterCommand(std::string_view name) = 0;
    [[nodiscard]] virtual bool hasCommand(std::string_view name) const = 0;
    [[nodiscard]] virtual std::vector<CommandMetadata> commands() const = 0;

    virtual bool submit(std::string_view line) = 0;
    virtual bool executeScript(std::string_view scriptSource) = 0;
    virtual void setCheatsEnabled(bool enabled) = 0;
    [[nodiscard]] virtual bool cheatsEnabled() const = 0;

    virtual void clearHistory() = 0;
    virtual void setHistoryLimit(std::size_t limit) = 0;
    [[nodiscard]] virtual const std::vector<std::string>& history() const = 0;

    [[nodiscard]] virtual CompletionResult suggestions(std::string_view line, std::size_t cursorPos) const = 0;

    [[nodiscard]] virtual const std::string& lastError() const = 0;
    virtual void clearError() = 0;

    virtual void writeLine(std::string_view text) = 0;
    virtual void clearOutput() = 0;
    [[nodiscard]] virtual const std::vector<std::string>& output() const = 0;
    [[nodiscard]] virtual std::string_view prompt() const = 0;
    virtual Utils::CallbackHandleUPtr addOutputListener(OutputListener listener) = 0;
};

} // namespace System
