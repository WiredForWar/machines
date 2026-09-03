#pragma once

#include "system/IConsole.hpp"

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace System
{

struct ConsoleConfig
{
    std::size_t historyLimit { 64U };
    std::size_t suggestionLimit { 10U };
    std::size_t outputLimit { 256U };
    std::string historyFilePath{};
    // Where the console reads the time from. Left unset, it uses the steady
    // clock; a caller that needs the passage of time to be its own to decide
    // supplies one.
    std::function<std::chrono::steady_clock::time_point()> clock{};
};

class Console : public IConsole
{
public:
    explicit Console(const ConsoleConfig& config = {});
    ~Console() override;

    bool registerCommand(const CommandMetadata& metadata, CommandHandler handler) override;
    bool
    registerCommand(const CommandMetadata& metadata, CommandHandler handler, CompletionProvider completer) override;
    bool unregisterCommand(std::string_view name) override;
    [[nodiscard]] bool hasCommand(std::string_view name) const override;
    [[nodiscard]] std::vector<CommandMetadata> commands() const override;

    bool submit(std::string_view line) override;
    bool executeScript(std::string_view scriptSource) override;

    void waitUntil(
        CompletionPredicate predicate,
        std::chrono::milliseconds timeout,
        std::string description,
        CompletionAction onCompleted) override;
    // The overload that takes no action is not virtual, and declaring the one
    // above would otherwise hide it from a caller holding a Console.
    using IConsole::waitUntil;
    [[nodiscard]] bool isBusy() const override;
    void cancelPending() override;
    void tick() override;
    [[nodiscard]] std::chrono::steady_clock::time_point now() const override;

    void setBlockModeEnabled(bool enabled) override;
    [[nodiscard]] bool blockModeEnabled() const override;

    void clearHistory() override;
    void setHistoryLimit(std::size_t limit) override;
    [[nodiscard]] const std::vector<std::string>& history() const override;

    void setCheatsEnabled(bool enabled) override;
    [[nodiscard]] bool cheatsEnabled() const override;

    void setDevModeEnabled(bool enabled) override;
    [[nodiscard]] bool devModeEnabled() const override;

    [[nodiscard]] CompletionResult suggestions(std::string_view line, std::size_t cursorPos) const override;

    [[nodiscard]] const std::string& lastError() const override;
    void clearError() override;
    void reportError(std::string message) override;

    void writeLine(std::string_view text) override;
    void clearOutput() override;
    [[nodiscard]] const std::vector<std::string>& output() const override;
    [[nodiscard]] std::string_view prompt() const override;
    Utils::CallbackHandleUPtr addOutputListener(OutputListener listener) override;

private:
    enum class EchoCommandLine
    {
        No,
        Yes,
    };

    struct CommandDefinition
    {
        CommandMetadata metadata{};
        CommandHandler handler{};
        CompletionProvider completer{};
    };

    using CommandMap = std::unordered_map<std::string, CommandDefinition>;

    struct QueuedCommand
    {
        std::string line{};
        EchoCommandLine echo{};
    };

    struct PendingCommand
    {
        CompletionPredicate predicate{};
        std::chrono::steady_clock::time_point deadline{};
        std::string description{};
        CompletionAction onCompleted{};
    };

    static std::vector<std::string> tokenize(std::string_view line);
    static bool isIdentifierToken(const std::string& token);

    bool parseArguments(
        const CommandMetadata& metadata,
        const std::vector<std::string>& tokenArguments,
        std::vector<ArgumentValue>& parsedArguments);
    bool convertToken(ArgumentType type, const std::string& token, ArgumentValue& value);
    bool executeCommand(std::string_view line, EchoCommandLine echo);
    void drainQueue();

    static bool isHistoryIgnored(std::string_view line);
    void appendHistoryEntry(const std::string& line);
    void loadHistory();
    void saveHistoryEntry(const std::string& line);
    void setError(std::string message);
    void appendOutputLine(std::string_view text);

    ConsoleConfig config_{};
    CommandMap commands_{};
    std::vector<std::string> history_{};

    class OutputListenerHandleImpl;

    struct OutputListenerEntry
    {
        const Utils::CallbackHandle* handle{};
        OutputListener callback{};
    };

    struct OutputListenerState
    {
        Console* console{};
    };

    void removeOutputListener(const Utils::CallbackHandle* handle);

    std::vector<std::string> output_{};
    std::string promptText_{};
    std::vector<OutputListenerEntry> outputListeners_{};
    std::shared_ptr<OutputListenerState> listenerState_{};
    std::string lastError_{};
    bool cheatsEnabled_{};
    bool devModeEnabled_{};
    bool blockModeEnabled_{};
    std::deque<QueuedCommand> queue_{};
    std::optional<PendingCommand> pending_{};
};

} // namespace System
