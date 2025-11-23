#include "Variable_p.hpp"

#include "utility/string.hpp"

#include <cstdint>

namespace Config
{

IVariable::IVariable(std::string_view name)
    : name_(name)
{
    Utils::replaceAll(&name_, "/", "\\");
}

std::string_view IVariable::name() const
{
    return name_;
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
