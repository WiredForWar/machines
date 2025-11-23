#pragma once

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

class IVariable
{
public:
    IVariable(std::string_view name);

    std::string_view name() const;

protected:
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

private:
    T defaultValue_{};
};

} // namespace Config
