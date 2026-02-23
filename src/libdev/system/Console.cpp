#include "Console.hpp"

#include "utility/String.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace System
{

Console::Console(const ConsoleConfig& config)
    : config_(config)
    , promptText_("> ")
{
    history_.reserve(std::min<std::size_t>(config_.historyLimit, history_.capacity()));
    output_.reserve(std::min<std::size_t>(config_.outputLimit, output_.capacity()));
    listenerState_ = std::make_shared<OutputListenerState>();
    listenerState_->console = this;
}

Console::~Console()
{
    if (listenerState_ != nullptr)
    {
        listenerState_->console = nullptr;
    }
    outputListeners_.clear();
}

bool Console::registerCommand(const CommandMetadata& metadata, CommandHandler handler)
{
    if (metadata.name.empty() || !handler)
    {
        setError("Command metadata invalid.");
        return false;
    }

    const std::string trimmed = Utils::trimWhitespace(metadata.name);
    if (trimmed.empty())
    {
        setError("Command name cannot be whitespace.");
        return false;
    }

    if (commands_.contains(trimmed))
    {
        setError("Command already registered: " + trimmed);
        return false;
    }

    CommandDefinition definition{};
    definition.metadata = metadata;
    definition.metadata.name = trimmed;
    definition.handler = std::move(handler);

    commands_.emplace(definition.metadata.name, std::move(definition));
    clearError();
    return true;
}

bool Console::unregisterCommand(std::string_view name)
{
    const std::string trimmed = Utils::trimWhitespace(name);
    const std::size_t erased = commands_.erase(trimmed);
    if (erased == 0)
    {
        setError("Command not found: " + std::string(trimmed));
        return false;
    }
    clearError();
    return true;
}

bool Console::hasCommand(std::string_view name) const
{
    return commands_.contains(std::string(name));
}

std::vector<Console::CommandMetadata> Console::commands() const
{
    std::vector<CommandMetadata> output;
    output.reserve(commands_.size());
    for (const CommandMap::value_type& entry : commands_)
    {
        output.push_back(entry.second.metadata);
    }
    return output;
}

bool Console::submit(std::string_view line)
{
    clearError();

    const std::string trimmed = Utils::trimWhitespace(line);
    if (trimmed.empty())
    {
        return false;
    }

    appendHistoryEntry(trimmed);

    std::string submittedLine = promptText_;
    submittedLine += trimmed;
    writeLine(submittedLine);

    const std::vector<std::string> tokens = tokenize(trimmed);
    if (tokens.empty())
    {
        setError("Unable to parse command.");
        return false;
    }

    const std::string commandName = tokens.front();
    const CommandMap::iterator commandIterator = commands_.find(commandName);
    if (commandIterator == commands_.end())
    {
        setError("Unknown command: " + commandName);
        return false;
    }

    const std::vector<std::string> tokenArguments(tokens.begin() + 1, tokens.end());
    std::vector<ArgumentValue> parsedArguments;
    if (!parseArguments(commandIterator->second.metadata, tokenArguments, parsedArguments))
    {
        return false;
    }

    CommandRequest request{};
    request.name = commandName;
    request.arguments = std::move(parsedArguments);
    request.rawLine = std::string(trimmed);
    commandIterator->second.handler(request, *this);
    return true;
}

bool Console::executeScript(std::string_view scriptSource)
{
    clearError();
    std::istringstream stream{std::string(scriptSource)};
    std::string line;
    bool allSucceeded = true;

    while (std::getline(stream, line))
    {
        const std::string trimmed = Utils::trimWhitespace(line);
        if (trimmed.empty() || trimmed.starts_with("//"))
        {
            continue;
        }

        if (!submit(trimmed))
        {
            allSucceeded = false;
        }
    }

    return allSucceeded;
}

void Console::clearHistory()
{
    history_.clear();
}

void Console::setHistoryLimit(std::size_t limit)
{
    config_.historyLimit = limit;
    if (history_.size() > limit)
    {
        history_.erase(history_.begin(), history_.begin() + static_cast<std::ptrdiff_t>(history_.size() - limit));
    }
}

const std::vector<std::string>& Console::history() const
{
    return history_;
}

const std::string& Console::lastError() const
{
    return lastError_;
}

void Console::clearError()
{
    lastError_.clear();
}

std::vector<std::string> Console::tokenize(std::string_view line)
{
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '"';

    for (std::size_t index = 0; index < line.size(); ++index)
    {
        const char ch = line[index];

        if (inQuotes)
        {
            if (ch == quoteChar)
            {
                inQuotes = false;
                tokens.push_back(current);
                current.clear();
            }
            else if (ch == '\\' && index + 1 < line.size())
            {
                current.push_back(line[++index]);
            }
            else
            {
                current.push_back(ch);
            }
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(ch)))
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        if (ch == '"' || ch == '\'')
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
            inQuotes = true;
            quoteChar = ch;
            continue;
        }

        current.push_back(ch);
    }

    if (!current.empty())
    {
        tokens.push_back(current);
    }

    return tokens;
}

bool Console::isIdentifierToken(const std::string& token)
{
    if (token.empty())
    {
        return false;
    }

    if (!std::isalpha(static_cast<unsigned char>(token.front())) && token.front() != '_')
    {
        return false;
    }

    return std::all_of(token.begin() + 1, token.end(), [](unsigned char ch) {
        return std::isalnum(ch) || ch == '_';
    });
}

bool Console::parseArguments(
    const CommandMetadata& metadata,
    const std::vector<std::string>& tokenArguments,
    std::vector<ArgumentValue>& parsedArguments)
{
    parsedArguments.clear();
    parsedArguments.reserve(metadata.arguments.size());

    std::size_t inputIndex = 0;
    for (const auto& spec : metadata.arguments)
    {
        ArgumentValue value{};
        value.type = spec.type;

        if (inputIndex < tokenArguments.size())
        {
            value.provided = true;
            if (!convertToken(spec.type, tokenArguments[inputIndex], value))
            {
                return false;
            }
            ++inputIndex;
        }
        else if (!spec.optional)
        {
            setError("Missing argument: " + spec.name);
            return false;
        }

        parsedArguments.push_back(std::move(value));
    }

    if (inputIndex < tokenArguments.size())
    {
        setError("Too many arguments for command.");
        return false;
    }

    return true;
}

bool Console::convertToken(ArgumentType type, const std::string& token, ArgumentValue& value)
{
    try
    {
        switch (type)
        {
        case ArgumentType::Boolean:
        {
            const std::string normalized = Utils::trimWhitespace(token);
            if (normalized == "1" || normalized == "true" || normalized == "TRUE" || normalized == "on")
            {
                value.value = true;
            }
            else if (normalized == "0" || normalized == "false" || normalized == "FALSE" || normalized == "off")
            {
                value.value = false;
            }
            else
            {
                setError("Invalid boolean: " + token);
                return false;
            }
            break;
        }
        case ArgumentType::Integer:
            value.value = std::stoll(token);
            break;
        case ArgumentType::Float:
            value.value = std::stod(token);
            break;
        case ArgumentType::Identifier:
            if (!isIdentifierToken(token))
            {
                setError("Invalid identifier: " + token);
                return false;
            }
            [[fallthrough]];
        case ArgumentType::String:
            value.value = token;
            break;
        }
    }
    catch (const std::exception&)
    {
        setError("Failed to parse argument: " + token);
        return false;
    }

    return true;
}

void Console::appendHistoryEntry(const std::string& line)
{
    if (line.empty())
    {
        return;
    }

    if (!history_.empty() && history_.back() == line)
        return;

    history_.push_back(line);

    if (history_.size() > config_.historyLimit)
    {
        history_.erase(history_.begin());
    }
}

void Console::setError(std::string message)
{
    lastError_ = std::move(message);
}

void Console::writeLine(std::string_view text)
{
    appendOutputLine(text);
}

const std::vector<std::string>& Console::output() const
{
    return output_;
}

std::string_view Console::prompt() const
{
    return promptText_;
}

class Console::OutputListenerHandleImpl : public Utils::CallbackHandle
{
public:
    explicit OutputListenerHandleImpl(std::shared_ptr<OutputListenerState> state)
        : state_(std::move(state))
    {
    }

    ~OutputListenerHandleImpl() override
    {
        if (const std::shared_ptr<OutputListenerState> shared = state_.lock())
        {
            if (shared->console != nullptr)
            {
                shared->console->removeOutputListener(this);
            }
        }
    }

private:
    std::weak_ptr<OutputListenerState> state_ {};
};

Utils::CallbackHandleUPtr Console::addOutputListener(OutputListener listener)
{
    if (listenerState_ == nullptr)
    {
        listenerState_ = std::make_shared<OutputListenerState>();
        listenerState_->console = this;
    }

    auto handle = std::make_unique<OutputListenerHandleImpl>(listenerState_);
    outputListeners_.push_back(OutputListenerEntry { handle.get(), std::move(listener) });
    return handle;
}

void Console::appendOutputLine(std::string_view text)
{
    if (config_.outputLimit > 0)
    {
        if (output_.size() == config_.outputLimit)
            output_.erase(output_.begin());

        output_.emplace_back(text);
    }

    for (const OutputListenerEntry& entry : outputListeners_)
    {
        entry.callback(text);
    }
}

void Console::removeOutputListener(const Utils::CallbackHandle* handle)
{
    std::erase_if(outputListeners_, [handle](const OutputListenerEntry& entry) { return entry.handle == handle; });
}

} // namespace System
