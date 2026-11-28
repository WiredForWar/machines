#pragma once

#include "gui/gui.hpp"

#include <cstddef>

namespace MachGui
{

// Where the list a drop down opens goes, and how much of it is on show.
struct DropDownPlacement
{
    // The list's boundary, in the same space as the anchor it came from.
    Gui::Box box;

    // How many items the box shows at once. Fewer than were asked for means the
    // rest are reached by scrolling. Zero means there is no room for a list.
    std::size_t visibleItems{};
};

// The part of an absolute root boundary visible in the window, expressed in
// that root's local coordinates. Centering a menu moves its origin without
// reducing the root's stored size, so its full size is not the visible area.
Gui::Box dropDownVisibleBounds(const Gui::Box& rootBoundary, Ren::Size windowSize);

// Place a list of itemCount items opening from anchor, the closed control, so that
// it stays inside bounds.
//
// The list opens downward from the anchor's top edge. If the whole list will not fit
// there it opens upward instead, ending at the anchor's bottom edge; and if it fits
// on neither side it takes the roomier one and shows as many items as that side
// holds.
//
// itemSpacing is the step from one item to the next. Items drawn overlapping have a
// spacing smaller than their height, so a run of them is shorter than
// itemCount * itemHeight.
DropDownPlacement dropDownPlacement(
    const Gui::Box& anchor,
    const Gui::Box& bounds,
    int itemHeight,
    int itemSpacing,
    std::size_t itemCount);

} // namespace MachGui
