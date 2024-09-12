#pragma once

#include <cstdint>

#include "base/prepost.hpp"

// Booleans are stored as 32-bit unsigned number which gives us some
// space to store extra info in a way compatible with old save files
class HackyFlagsPacker
{
    using BooleanStorage = uint32_t;
    static constexpr std::size_t Capacity = (sizeof(BooleanStorage) * 8 - 2);
    static constexpr BooleanStorage HackyPackIndicator = 3 << Capacity;

public:
    uint32_t value = HackyPackIndicator;
    void unpackBoolean(int index, bool* boolean) const
    {
        PRE(index < Capacity);
        if (index == 0 || (value & HackyPackIndicator))
        {
            *boolean = value & (1 << index);
        }
    }

    void packBoolean(int index, bool boolean)
    {
        PRE(index < Capacity);
        if (boolean)
            value |= 1 << index;
        else
            value &= ~(1 << index);
    }
};
