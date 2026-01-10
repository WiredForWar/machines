#pragma once

#include "system/IConsole.hpp"

#include <unordered_map>

namespace System
{

struct ConsoleConfig
{
    std::size_t historyLimit { 64U };
    std::size_t suggestionLimit { 10U };
};

class Console : public IConsole
{
public:
    explicit Console(const ConsoleConfig& config = {});

    bool registerCommand(const CommandMetadata& metadata, CommandHandler handler) override;
    bool unregisterCommand(std::string_view name) override;
    [[nodiscard]] bool hasCommand(std::string_view name) const override;
    [[nodiscard]] std::vector<CommandMetadata> commands() const override;

    bool submit(std::string_view line) override;
    bool executeScript(std::string_view scriptSource) override;

    void clearHistory() override;
    void setHistoryLimit(std::size_t limit) override;
    [[nodiscard]] const std::vector<std::string>& history() const override;

    [[nodiscard]] std::vector<std::string> suggestions(std::string_view prefix) const override;

    [[nodiscard]] const std::string& lastError() const override;
    void clearError() override;

private:
    struct CommandDefinition
    {
        CommandMetadata metadata{};
        CommandHandler handler{};
    };

    using CommandMap = std::unordered_map<std::string, CommandDefinition>;

    static std::vector<std::string> tokenize(std::string_view line);
    static bool isIdentifierToken(const std::string& token);

    bool parseArguments(
        const CommandMetadata& metadata,
        const std::vector<std::string>& tokenArguments,
        std::vector<ArgumentValue>& parsedArguments);
    bool convertToken(ArgumentType type, const std::string& token, ArgumentValue& value);

    void appendHistoryEntry(const std::string& line);
    void setError(std::string message);

    ConsoleConfig config_{};
    CommandMap commands_{};
    std::vector<std::string> history_{};
    std::string lastError_{};
};

} // namespace System
