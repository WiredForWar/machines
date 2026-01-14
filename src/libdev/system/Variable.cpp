#include "Variable_p.hpp"

#include "utility/string.hpp"

#include <cstdint>
#include <unordered_map>

namespace Config
{

namespace
{

class ConfigManager
{
public:
    using ChangesListener = IVariable::ChangesListener;
    class ChangesListenerHandleImpl;

    ConfigManager();

    Utils::HandleWithTriggerUPtr addListener(std::string_view name, ChangesListener listener);
    void removeListener(const Utils::CallbackHandle* handle);

    void onVariableChanged(std::string_view variable);

private:
    struct ListenerState
    {
        ConfigManager* manager_{};
    };

    class string_hash
    {
    public:
        using is_transparent = void;
        [[nodiscard]] size_t operator()(std::string_view v) const { return std::hash<std::string_view> {}(v); }
        [[nodiscard]] size_t operator()(const std::string& v) const { return std::hash<std::string> {}(v); }
    };

    std::shared_ptr<ListenerState> listenerState_{};
    std::unordered_multimap<std::string, const ChangesListenerHandleImpl*, string_hash, std::equal_to<>> listeners_{};
};

class ConfigManager::ChangesListenerHandleImpl : public Utils::HandleWithTrigger
{
public:
    explicit ChangesListenerHandleImpl(std::shared_ptr<ListenerState> state, ChangesListener listener)
        : state_(std::move(state))
        , callback_(std::move(listener))
    {
    }

    void trigger() const override
    {
        callback_();
    }

    ~ChangesListenerHandleImpl() override
    {
        if (const std::shared_ptr<ListenerState> shared = state_.lock())
        {
            if (shared->manager_ != nullptr)
            {
                shared->manager_->removeListener(this);
            }
        }
    }

private:
    std::weak_ptr<ListenerState> state_{};
    ChangesListener callback_{};
};

ConfigManager::ConfigManager()
{
    listenerState_ = std::make_shared<ListenerState>();
    listenerState_->manager_ = this;
}

void ConfigManager::removeListener(const Utils::CallbackHandle* handle)
{
    for (auto it = listeners_.begin(); it != listeners_.end();)
    {
        if (it->second == handle)
        {
            it = listeners_.erase(it);
            continue;
        }

        ++it;
    }
}

void ConfigManager::onVariableChanged(std::string_view variable)
{
    const auto [begin, end] = listeners_.equal_range(variable);
    for (auto it = begin; it != end; ++it)
    {
        it->second->trigger();
    }
}

Utils::HandleWithTriggerUPtr ConfigManager::addListener(std::string_view name, ChangesListener listener)
{
    if (!listenerState_)
    {
        listenerState_ = std::make_shared<ListenerState>();
        listenerState_->manager_ = this;
    }

    auto handle = std::make_unique<ChangesListenerHandleImpl>(listenerState_, std::move(listener));
    listeners_.emplace(std::string(name), handle.get());
    return handle;
}

std::unique_ptr<ConfigManager> configManager;

} // namespace

void initConfigManager()
{
    configManager = std::make_unique<ConfigManager>();
}

void cleanUpConfigManager()
{
    configManager.reset();
}

IVariable::IVariable(std::string_view name)
    : name_(name)
{
    Utils::replaceAll(&name_, "/", "\\");
}

std::string_view IVariable::name() const
{
    return name_;
}

Utils::HandleWithTriggerUPtr IVariable::addListener(ChangesListener listener)
{
    return configManager->addListener(name(), std::move(listener));
}

void IVariable::onChanged()
{
    PRE(configManager.get());
    configManager->onVariableChanged(name());
}

template <>
std::string Impl::toString(const int32_t& value)
{
    return std::to_string(value);
}

template <>
std::optional<int32_t> Impl::toValue(const std::string& asString)
{
    return std::atoi(asString.c_str());
}

template class Variable<int32_t>;

template <>
std::string Impl::toString(const uint32_t& value)
{
    return std::to_string(value);
}

template <>
std::optional<uint32_t> Impl::toValue(const std::string& asString)
{
    return std::atoi(asString.c_str());
}

template class Variable<uint32_t>;

template <>
std::string Impl::toString(const bool& value)
{
    return value ? "1" : "0";
}

template <>
std::optional<bool> Impl::toValue(const std::string& asString)
{
    if (asString == "1")
        return true;
    if (asString == "0")
        return false;

    return std::nullopt;
}

template class Variable<bool>;

} // namespace Config
