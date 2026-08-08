#include "machgui/DropDownPlacement.hpp"

#include <algorithm>

namespace MachGui
{

namespace
{

// How many items of the given geometry are drawn without reaching past `space`.
std::size_t itemsFitting(int space, int itemHeight, int itemSpacing)
{
    if (space < itemHeight)
        return 0;

    if (itemSpacing <= 0)
        return 1;

    return 1 + static_cast<std::size_t>((space - itemHeight) / itemSpacing);
}

int heightForItems(std::size_t items, int itemHeight, int itemSpacing)
{
    return static_cast<int>(items - 1) * itemSpacing + itemHeight;
}

} // namespace

DropDownPlacement dropDownPlacement(
    const Gui::Box& anchor,
    const Gui::Box& bounds,
    int itemHeight,
    int itemSpacing,
    std::size_t itemCount)
{
    if (itemCount == 0)
        return {};

    const int boundsTop = static_cast<int>(bounds.minCorner().y());
    const int boundsBottom = static_cast<int>(bounds.maxCorner().y());

    // The edges the list grows from. Held inside the bounds so that an anchor
    // hanging over an edge cannot carry the list out with it.
    const int downwardStart = std::clamp(static_cast<int>(anchor.minCorner().y()), boundsTop, boundsBottom);
    const int upwardEnd = std::clamp(static_cast<int>(anchor.maxCorner().y()), boundsTop, boundsBottom);

    const std::size_t fitsBelow
        = std::min(itemCount, itemsFitting(boundsBottom - downwardStart, itemHeight, itemSpacing));
    const std::size_t fitsAbove = std::min(itemCount, itemsFitting(upwardEnd - boundsTop, itemHeight, itemSpacing));

    // Downward is preferred, and is given up only when the whole list will not go
    // there and more of it fits above.
    const bool openDownward = fitsBelow == itemCount || fitsBelow >= fitsAbove;
    const std::size_t visibleItems = openDownward ? fitsBelow : fitsAbove;

    if (visibleItems == 0)
        return {};

    const int height = heightForItems(visibleItems, itemHeight, itemSpacing);

    DropDownPlacement placement;
    placement.box = Gui::Box(
        Gui::Coord(anchor.minCorner().x(), openDownward ? downwardStart : upwardEnd - height),
        anchor.maxCorner().x() - anchor.minCorner().x(),
        height);
    placement.visibleItems = visibleItems;

    return placement;
}

} // namespace MachGui
