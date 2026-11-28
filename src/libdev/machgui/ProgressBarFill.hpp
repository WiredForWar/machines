#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace MachGui
{
inline int progressBarFillWidth(int width, std::size_t done, std::size_t total,
    double lower, double upper)
{
    const double fraction = total ? std::clamp(double(done) / double(total), 0.0, 1.0) : 0.0;
    const double progress = std::clamp(lower + fraction * (upper - lower), 0.0, 1.0);
    return int(std::lround(std::max(0, width) * progress));
}
}
