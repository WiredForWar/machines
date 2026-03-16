#pragma once

#include "Variable.hpp"

#include "system/Registry.hpp"

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
    if (!value_.has_value() && !name_.empty())
    {
        std::string asString;
        SysRegistry::instance().queryValueNoRecord(name_, asString);
        if (!asString.empty())
        {
            value_ = Impl::toValue<T>(asString);
        }
    }

    if (!value_.has_value())
        value_ = defaultValue_;

    return value_.value();
}

template<typename T>
void Variable<T>::set(const T& value)
{
    write(value);
    onChanged();
}

template <typename T>
void Variable<T>::writeBack()
{
    write(get());
}

template <typename T>
void Variable<T>::write(const T& value)
{
    SysRegistry::instance().setValue(name_, Impl::toString<T>(value));
    value_ = value;
}

} // namespace Config
