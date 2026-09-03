#pragma once

#include <chrono>
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
        bool repeating{};
        std::string description{};
    };

    struct CommandMetadata
    {
        std::string name{};
        std::string description{};
        std::vector<ArgumentSpec> arguments{};
        bool cheat{};
        bool devOnly{};
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
    using CompletionPredicate = std::function<bool()>;
    using CompletionAction = std::function<void()>;

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

    // Takes a line to run. A line offered while the console is busy is held
    // back and run in its turn, so what comes back says the console accepted
    // the line rather than that the command succeeded.
    virtual bool submit(std::string_view line) = 0;
    virtual bool executeScript(std::string_view scriptSource) = 0;

    // For a command handler to say that the work it started is not over when
    // it returns. Nothing else runs until the predicate first answers true;
    // if the timeout passes first the console says so, and drops whatever was
    // waiting its turn. Does nothing at all while blocking mode is off, so a
    // handler need not ask about the mode to name what it waits for.
    //
    // onCompleted is how a command answers about work it could not answer about
    // when its handler ran -- a measurement, say, that has no result until the
    // frames are in. It runs when the predicate is satisfied and only then:
    // never on the timeout, never on a cancel, and never when the mode is off
    // and there is no wait to follow.
    virtual void waitUntil(
        CompletionPredicate predicate,
        std::chrono::milliseconds timeout,
        std::string description,
        CompletionAction onCompleted)
        = 0;

    // For a wait with nothing to say afterwards.
    void waitUntil(CompletionPredicate predicate, std::chrono::milliseconds timeout, std::string description)
    {
        waitUntil(std::move(predicate), timeout, std::move(description), CompletionAction{});
    }

    // Whether a command is still running, or waiting its turn behind one.
    [[nodiscard]] virtual bool isBusy() const = 0;

    // Stops waiting and drops every line still waiting its turn. Work that a
    // command has already set going carries on; only the waiting ends.
    virtual void cancelPending() = 0;

    // Lets the console finish a wait and run what is queued behind it.
    // Expected once a frame.
    virtual void tick() = 0;

    // The clock a wait's deadline is measured against.
    [[nodiscard]] virtual std::chrono::steady_clock::time_point now() const = 0;

    // Whether a command that finishes later than its handler holds the input
    // back until it is done.
    virtual void setBlockModeEnabled(bool enabled) = 0;
    [[nodiscard]] virtual bool blockModeEnabled() const = 0;

    virtual void setCheatsEnabled(bool enabled) = 0;
    [[nodiscard]] virtual bool cheatsEnabled() const = 0;

    virtual void setDevModeEnabled(bool enabled) = 0;
    [[nodiscard]] virtual bool devModeEnabled() const = 0;

    virtual void clearHistory() = 0;
    virtual void setHistoryLimit(std::size_t limit) = 0;
    [[nodiscard]] virtual const std::vector<std::string>& history() const = 0;

    [[nodiscard]] virtual CompletionResult suggestions(std::string_view line, std::size_t cursorPos) const = 0;

    [[nodiscard]] virtual const std::string& lastError() const = 0;
    virtual void clearError() = 0;

    // For a command handler to say it could not do what was asked. The message
    // is printed and becomes lastError(), and the submission that ran the
    // handler reads as refused -- so a caller that is driving the console from
    // outside learns of the failure instead of only a line of output.
    virtual void reportError(std::string message) = 0;

    virtual void writeLine(std::string_view text) = 0;
    virtual void clearOutput() = 0;
    [[nodiscard]] virtual const std::vector<std::string>& output() const = 0;
    [[nodiscard]] virtual std::string_view prompt() const = 0;
    virtual Utils::CallbackHandleUPtr addOutputListener(OutputListener listener) = 0;
};

} // namespace System
