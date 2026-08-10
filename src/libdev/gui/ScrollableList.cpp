/*
 * S C R O L I S T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "gui/ScrollableList.hpp"
#include "gui/Event.hpp"
#include "gui/ListObserver.hpp"

GuiSimpleScrollableList::GuiSimpleScrollableList(
    GuiDisplayable* pParent,
    const Gui::Box& box,
    size_t horizontalSpacing,
    size_t verticalSpacing,
    size_t scrollInc)
    : GuiDisplayable(pParent, box)
{
    observers_.reserve(64);

    horizontalSpacing_ = horizontalSpacing;
    verticalSpacing_ = verticalSpacing;
    scrollInc_ = scrollInc;

    // Work out how many positions there are that things can be displayed in
    numPositions_ = 0;

    Gui::Coord relPos(0, 0);
    bool endOfDisplayableAreas = false;

    while (! endOfDisplayableAreas)
    {
        ++numPositions_;
        relPos.x(relPos.x() + horizontalSpacing_);
        if (relPos.x() >= width())
        {
            relPos.x(0);

            relPos.y(relPos.y() + verticalSpacing_);

            if (relPos.y() >= height())
            {
                endOfDisplayableAreas = true;
            }
        }
    }

    TEST_INVARIANT;
}

GuiSimpleScrollableList::~GuiSimpleScrollableList()
{
    TEST_INVARIANT;

    // If there are any observers still attached then tell them I am being deleted. It
    // is their responsibility to make sure they don't call an invalid list.
    for (GuiListObserver *observer : observers_)
        observer->listDeleted();
}

void GuiSimpleScrollableList::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const GuiSimpleScrollableList& t)
{
    o << "GuiSimpleScrollableList " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "GuiSimpleScrollableList " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

bool GuiSimpleScrollableList::canScrollFoward() const
{
    return canScrollFoward_;
}

bool GuiSimpleScrollableList::canScrollBackward() const
{
    return canScrollBackward_;
}

bool GuiSimpleScrollableList::canScroll() const
{
    return (canScrollFoward_ || canScrollBackward_);
}

void GuiSimpleScrollableList::scrollFoward()
{
    PRE(canScrollFoward());

    scrollOffset_ += scrollInc_;

    childrenUpdated();
}

void GuiSimpleScrollableList::scrollBackward()
{
    PRE(canScrollBackward());

    if (scrollOffset_ != 0)
        scrollOffset_ -= scrollInc_;

    childrenUpdated();
}

void GuiSimpleScrollableList::scrollTo(size_t scrollPos)
{
    PRE(canScroll());
    PRE(scrollPos < numListItems());

    scrollOffset_ = scrollPos;

    childrenUpdated();
}

void GuiSimpleScrollableList::updateInfo()
{
    size_t numChildren = children().size();

    unsigned maxOffset = (numChildren < numPositions_) ? 0 : numChildren - numPositions_ + scrollInc_ - 1;

    if (scrollOffset_ > maxOffset)
        scrollOffset_ = maxOffset;

    scrollOffset_ -= scrollOffset_ % scrollInc_; // make sure offset is multiple of scroll increment.

    canScrollBackward_ = (scrollOffset_ != 0);
    canScrollFoward_ = numChildren - scrollOffset_ > numPositions_;
}

void GuiSimpleScrollableList::childrenUpdated()
{
    updateInfo();

    // Reposition all the children, making the visible if they fall into the displayable area
    Gui::Coord relPos(0, 0);
    bool endOfDisplayableChildren = false;

    std::size_t scrollOffsetCountDown = scrollOffset_;

    for (GuiDisplayable* child : children())
    {
        if (scrollOffsetCountDown) // Have we reached visible children yet?
        { // No
            --scrollOffsetCountDown;
            child->setVisible(false);
        }
        else
        { // Yes
            child->setVisible(!endOfDisplayableChildren);

            if (!endOfDisplayableChildren)
                positionChildRelative(child, relPos); // Only reposition if child is visible

            relPos.x(relPos.x() + horizontalSpacing_);
            if (relPos.x() >= width())
            {
                relPos.x(0);

                relPos.y(relPos.y() + verticalSpacing_);

                if (relPos.y() >= height())
                {
                    endOfDisplayableChildren = true;
                }
            }
        }
    }

    // Tell any observers that I have changed.
    notifyObservers();

    changed();
}

void GuiSimpleScrollableList::childAdded()
{
    updateInfo();

    if (children().size() - scrollOffset_ > numPositions_)
    {
        // no need to do anything, the child does not fall into the viewable area
        children().back()->setVisible(false);
    }
    else
    {
        size_t drawPos = children().size() - scrollOffset_;

        Gui::Coord relPos(0, 0);

        while (--drawPos)
        {
            relPos.x(relPos.x() + horizontalSpacing_);
            if (relPos.x() >= width())
            {
                relPos.x(0);

                relPos.y(relPos.y() + verticalSpacing_);
            }
        }

        positionChildRelative(children().back(), relPos);
    }

    // Tell any observers that I have changed.
    notifyObservers();
}

void GuiSimpleScrollableList::doDisplay()
{
}

size_t GuiSimpleScrollableList::scrollIncrement() const
{
    return scrollInc_;
}

size_t GuiSimpleScrollableList::scrollOffset() const
{
    return scrollOffset_;
}

size_t GuiSimpleScrollableList::visiblePositions() const
{
    return numPositions_;
}

size_t GuiSimpleScrollableList::numListItems() const
{
    return children().size();
}

void GuiSimpleScrollableList::notifyMe(GuiListObserver* pObserver)
{
    observers_.push_back(pObserver);
}

void GuiSimpleScrollableList::dontNotifyMe(GuiListObserver* pObserver)
{
    ctl_pvector<GuiListObserver>::iterator iter = find(observers_.begin(), observers_.end(), pObserver);

    ASSERT(iter != observers_.end(), "couldn't find pObserver in observers_ list");

    observers_.erase(iter);
}

void GuiSimpleScrollableList::notifyObservers()
{
    for (GuiListObserver *observer : observers_)
        observer->listUpdated();
}

void GuiSimpleScrollableList::doHandleMouseScrollEvent(GuiMouseEvent* event)
{
    switch (event->scrollDirection())
    {
        case Gui::ScrollState::NO_SCROLL:
            break;

        case Gui::ScrollState::SCROLL_UP:
            if (canScrollBackward())
            {
                scrollBackward();
                event->accept();
            }
            break;
        case Gui::ScrollState::SCROLL_DOWN:
            if (canScrollFoward())
            {
                scrollFoward();
                event->accept();
            }
            break;
    }
}

// Scroll to the end of the list
void GuiSimpleScrollableList::scrollToEnd()
{
    while (canScrollFoward())
        scrollFoward();
}

// Scroll to the beginning of the list
void GuiSimpleScrollableList::scrollToBegin()
{
    while (canScrollBackward())
        scrollBackward();
}
