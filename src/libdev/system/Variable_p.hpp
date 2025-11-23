#include "Variable.hpp"

#include "system/registry.hpp"

namespace Config
{

template<typename T>
Variable<T>::Variable(std::string_view name, T defaultValue)
    : IVariable(name)
    , defaultValue_(defaultValue)
{
}

template<typename T>
T Variable<T>::get() const
{
    std::string asString;

    SysRegistry::instance().queryValueNoRecord(name_, asString);
    if (asString.empty())
        return defaultValue_;

    return Impl::toValue<T>(asString).value_or(defaultValue_);
}

template<typename T>
void Variable<T>::set(const T& value)
{
    SysRegistry::instance().setValue(name_, Impl::toString<T>(value));
}

} // namespace Config
