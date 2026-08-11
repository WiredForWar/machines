#include "gui/Layout.hpp"

#include "gui/Displayable.hpp"

#include <algorithm>

//////////////////////////////////////////////////////////////////////

GuiLayout::GuiLayout() = default;

GuiLayout::~GuiLayout() = default;

//////////////////////////////////////////////////////////////////////

GuiBoxLayout::GuiBoxLayout(Direction direction, std::size_t spacing)
    : direction_(direction)
    , spacing_(spacing)
{
}

GuiBoxLayout::~GuiBoxLayout() = default;

GuiBoxLayout::Direction GuiBoxLayout::direction() const
{
    return direction_;
}

std::size_t GuiBoxLayout::spacing() const
{
    return spacing_;
}

void GuiBoxLayout::setSpacing(std::size_t spacing)
{
    spacing_ = spacing;
}

void GuiBoxLayout::add(GuiDisplayable* pDisplayable)
{
    PRE(pDisplayable != nullptr);

    entries_.push_back(Entry{ .pDisplayable = pDisplayable });
}

void GuiBoxLayout::add(std::unique_ptr<GuiLayout> pLayout)
{
    PRE(pLayout != nullptr);

    entries_.push_back(Entry{ .pLayout = std::move(pLayout) });
}

void GuiBoxLayout::addSpacing(std::size_t length)
{
    entries_.push_back(Entry{ .length = length });
}

std::size_t GuiBoxLayout::count() const
{
    return entries_.size();
}

//////////////////////////////////////////////////////////////////////

MATHEX_SCALAR GuiBoxLayout::along(const Gui::Size& size) const
{
    return direction_ == Direction::VERTICAL ? size.height() : size.width();
}

MATHEX_SCALAR GuiBoxLayout::across(const Gui::Size& size) const
{
    return direction_ == Direction::VERTICAL ? size.width() : size.height();
}

Gui::Size GuiBoxLayout::sized(MATHEX_SCALAR alongExtent, MATHEX_SCALAR acrossExtent) const
{
    if (direction_ == Direction::VERTICAL)
        return Gui::Size(acrossExtent, alongExtent);

    return Gui::Size(alongExtent, acrossExtent);
}

// static
Gui::Size GuiBoxLayout::entrySize(const Entry& entry, Wanted wanted)
{
    if (entry.pDisplayable)
    {
        return wanted == Wanted::MINIMUM ? entry.pDisplayable->minimumSizeHint() : entry.pDisplayable->sizeHint();
    }

    if (entry.pLayout)
    {
        return wanted == Wanted::MINIMUM ? entry.pLayout->minimumSizeHint() : entry.pLayout->sizeHint();
    }

    // A gap takes no room across the layout.
    return Gui::Size(entry.length, entry.length);
}

Gui::Size GuiBoxLayout::totalSize(Wanted wanted) const
{
    MATHEX_SCALAR alongExtent = 0;
    MATHEX_SCALAR acrossExtent = 0;

    for (const Entry& entry : entries_)
    {
        const Gui::Size size = entrySize(entry, wanted);

        alongExtent += along(size);

        // A gap does not widen the layout.
        if (entry.pDisplayable || entry.pLayout)
            acrossExtent = std::max(acrossExtent, across(size));
    }

    if (entries_.size() > 1)
        alongExtent += spacing_ * (entries_.size() - 1);

    return sized(alongExtent, acrossExtent);
}

Gui::Size GuiBoxLayout::sizeHint() const
{
    return totalSize(Wanted::PREFERRED);
}

Gui::Size GuiBoxLayout::minimumSizeHint() const
{
    return totalSize(Wanted::MINIMUM);
}

void GuiBoxLayout::setGeometry(const Gui::Box& box)
{
    const MATHEX_SCALAR acrossExtent = across(box.size());
    MATHEX_SCALAR offset = 0;

    for (const Entry& entry : entries_)
    {
        const MATHEX_SCALAR alongExtent = along(entrySize(entry, Wanted::PREFERRED));

        if (entry.pDisplayable || entry.pLayout)
        {
            Gui::Coord topLeft = box.minCorner();
            if (direction_ == Direction::VERTICAL)
                topLeft.y(topLeft.y() + offset);
            else
                topLeft.x(topLeft.x() + offset);

            const Gui::Box entryBox(topLeft, sized(alongExtent, acrossExtent));

            if (entry.pDisplayable)
                entry.pDisplayable->setRelativeBoundary(entryBox);
            else
                entry.pLayout->setGeometry(entryBox);
        }

        offset += alongExtent + spacing_;
    }
}
