#include "render/DisplayModeCatalogue.hpp"

#include <algorithm>

namespace Ren
{

namespace
{

const std::vector<int>& noRefreshRates()
{
    static const std::vector<int> empty;
    return empty;
}

// Larger first, and the wider of two of the same area first, so that the order is
// the same whatever order the modes arrived in.
bool isBetterResolution(Size lhs, Size rhs)
{
    const int lhsPixels = lhs.width * lhs.height;
    const int rhsPixels = rhs.width * rhs.height;

    if (lhsPixels != rhsPixels)
        return lhsPixels > rhsPixels;

    return lhs.width > rhs.width;
}

} // namespace

DisplayModeCatalogue::DisplayModeCatalogue(const std::vector<IWindowAdapter::DisplayMode>& modes)
{
    for (const IWindowAdapter::DisplayMode& mode : modes)
    {
        const Size resolution(mode.width, mode.height);

        auto it = std::find(resolutions_.begin(), resolutions_.end(), resolution);
        if (it == resolutions_.end())
        {
            resolutions_.push_back(resolution);
            refreshRates_.emplace_back();
            it = resolutions_.end() - 1;
        }

        std::vector<int>& rates = refreshRates_[it - resolutions_.begin()];
        if (std::find(rates.begin(), rates.end(), mode.refreshRate) == rates.end())
            rates.push_back(mode.refreshRate);
    }

    // Sort the two lists together, by moving an order worked out on the
    // resolutions over to the rates that go with them.
    std::vector<std::size_t> order(resolutions_.size());
    for (std::size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    std::sort(
        order.begin(),
        order.end(),
        [this](std::size_t lhs, std::size_t rhs) { return isBetterResolution(resolutions_[lhs], resolutions_[rhs]); });

    std::vector<Size> sortedResolutions;
    std::vector<std::vector<int>> sortedRefreshRates;
    sortedResolutions.reserve(order.size());
    sortedRefreshRates.reserve(order.size());

    for (const std::size_t index : order)
    {
        sortedResolutions.push_back(resolutions_[index]);
        sortedRefreshRates.push_back(std::move(refreshRates_[index]));

        std::vector<int>& rates = sortedRefreshRates.back();
        std::sort(rates.begin(), rates.end(), std::greater<int>());
    }

    resolutions_ = std::move(sortedResolutions);
    refreshRates_ = std::move(sortedRefreshRates);
}

bool DisplayModeCatalogue::isEmpty() const
{
    return resolutions_.empty();
}

const std::vector<Size>& DisplayModeCatalogue::resolutions() const
{
    return resolutions_;
}

bool DisplayModeCatalogue::offers(Size resolution) const
{
    return std::find(resolutions_.begin(), resolutions_.end(), resolution) != resolutions_.end();
}

const std::vector<int>& DisplayModeCatalogue::refreshRates(Size resolution) const
{
    const auto it = std::find(resolutions_.begin(), resolutions_.end(), resolution);
    if (it == resolutions_.end())
        return noRefreshRates();

    return refreshRates_[it - resolutions_.begin()];
}

int DisplayModeCatalogue::bestRefreshRate(Size resolution) const
{
    const std::vector<int>& rates = refreshRates(resolution);

    return rates.empty() ? 0 : rates.front();
}

} // namespace Ren
