#pragma once

#include <type_traits>

template <typename EClass> class TFlag
{
public:
    using UnderlyingT = std::underlying_type_t<EClass>;
    TFlag(UnderlyingT value);

    TFlag(const TFlag<EClass>& value) = default;
    TFlag() = default;

    operator bool() const;
    TFlag operator&(EClass value) const;
    TFlag operator&(TFlag value) const;

private:
    UnderlyingT mValue {};
};

template <typename EClass>
TFlag<EClass>::TFlag(UnderlyingT value)
    : mValue(value)
{
}

template <typename EClass> TFlag<EClass>::operator bool() const
{
    return mValue != 0;
}

template <typename EClass> TFlag<EClass> TFlag<EClass>::operator&(EClass value) const
{
    return mValue & static_cast<UnderlyingT>(value);
}

template <typename EClass> TFlag<EClass> TFlag<EClass>::operator&(TFlag value) const
{
    return mValue & static_cast<UnderlyingT>(value);
}
