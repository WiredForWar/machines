#pragma once

#include "utility/CallbackHandle.hpp"

#include <functional>
#include <optional>
#include <string>

namespace Config
{

namespace Impl
{

template <typename T>
extern std::string toString(const T& value);

template <typename T>
extern std::optional<T> toValue(const std::string& asString);

} // namespace Impl

void initConfigManager();
void cleanUpConfigManager();

class IVariable
{
public:
    IVariable(std::string_view name);

    std::string_view name() const;

    using ChangesListener = std::function<void()>;
    Utils::HandleWithTriggerUPtr addListener(ChangesListener listener);

protected:
    void onChanged();

    std::string name_;
};

template <typename T>
class Variable : public IVariable
{
public:
    using value_type = T;

    Variable(std::string_view name, T defaultValue);

    void set(const T& value);
    T get() const;

    // Ensure that that option exists in the settings file.
    void writeBack();

private:
    void write(const T& value);

    T defaultValue_{};
    mutable std::optional<T> value_;
};

} // namespace Config
