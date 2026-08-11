#include <gtest/gtest.h>

#include "TestDisplayable.hpp"

#include "gui/Layout.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace
{

using Direction = GuiBoxLayout::Direction;

// A displayable of a fixed preferred size, placed out of the way so that a
// layout moving it is unmistakable.
GuiTest::Displayable* itemOfSize(GuiDisplayable* pParent, const Gui::Size& size)
{
    auto* pItem = new GuiTest::Displayable(pParent, Gui::Box(Gui::Coord(0, 0), size));
    pItem->setSizeHint(size);
    return pItem;
}

} // namespace

//////////////////////////////////////////////////////////////////////////////

TEST(GuiLayoutTests, SizeHint_Vertical_AddsTheHeightsAndTakesTheWidestWidth)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 5);

    layout.add(itemOfSize(&root, Gui::Size(30, 10)));
    layout.add(itemOfSize(&root, Gui::Size(70, 20)));
    layout.add(itemOfSize(&root, Gui::Size(50, 10)));

    // Heights 10 + 20 + 10, and 5 of spacing in each of the two gaps.
    EXPECT_EQ(Gui::Size(70, 50), layout.sizeHint());
}

TEST(GuiLayoutTests, SizeHint_Horizontal_AddsTheWidthsAndTakesTheTallestHeight)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::HORIZONTAL, 4);

    layout.add(itemOfSize(&root, Gui::Size(30, 10)));
    layout.add(itemOfSize(&root, Gui::Size(20, 25)));

    EXPECT_EQ(Gui::Size(54, 25), layout.sizeHint());
}

TEST(GuiLayoutTests, SizeHint_WhenEmpty_IsNothing)
{
    GuiBoxLayout layout(Direction::VERTICAL, 5);

    EXPECT_EQ(0u, layout.count());
    EXPECT_EQ(Gui::Size(0, 0), layout.sizeHint());
}

TEST(GuiLayoutTests, SizeHint_OneEntry_HasNoSpacingToAdd)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 5);

    layout.add(itemOfSize(&root, Gui::Size(30, 10)));

    EXPECT_EQ(Gui::Size(30, 10), layout.sizeHint());
}

TEST(GuiLayoutTests, MinimumSizeHint_IsTakenFromTheEntriesMinimums)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 0);

    auto* pFirst = itemOfSize(&root, Gui::Size(30, 10));
    pFirst->setMinimumSizeHint(Gui::Size(10, 4));
    auto* pSecond = itemOfSize(&root, Gui::Size(70, 20));
    pSecond->setMinimumSizeHint(Gui::Size(20, 6));

    layout.add(pFirst);
    layout.add(pSecond);

    EXPECT_EQ(Gui::Size(70, 30), layout.sizeHint());
    EXPECT_EQ(Gui::Size(20, 10), layout.minimumSizeHint());
}

//////////////////////////////////////////////////////////////////////////////

TEST(GuiLayoutTests, SetGeometry_Vertical_StacksTheEntriesFromTheTop)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 5);

    auto* pFirst = itemOfSize(&root, Gui::Size(30, 10));
    auto* pSecond = itemOfSize(&root, Gui::Size(30, 20));
    auto* pThird = itemOfSize(&root, Gui::Size(30, 10));
    layout.add(pFirst);
    layout.add(pSecond);
    layout.add(pThird);

    layout.setGeometry(Gui::Box(Gui::Coord(100, 200), Gui::Size(80, 60)));

    EXPECT_EQ(Gui::Coord(100, 200), pFirst->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Coord(100, 215), pSecond->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Coord(100, 240), pThird->relativeBoundary().minCorner());
}

TEST(GuiLayoutTests, SetGeometry_Vertical_WidensTheEntriesToTheBox)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 0);

    auto* pItem = itemOfSize(&root, Gui::Size(30, 10));
    layout.add(pItem);

    layout.setGeometry(Gui::Box(Gui::Coord(0, 0), Gui::Size(80, 60)));

    // Across the layout an entry fills the box; along it, it keeps its own size.
    EXPECT_EQ(Gui::Size(80, 10), pItem->relativeBoundary().size());
}

TEST(GuiLayoutTests, SetGeometry_Horizontal_PlacesTheEntriesLeftToRight)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::HORIZONTAL, 3);

    auto* pFirst = itemOfSize(&root, Gui::Size(30, 10));
    auto* pSecond = itemOfSize(&root, Gui::Size(20, 10));
    layout.add(pFirst);
    layout.add(pSecond);

    layout.setGeometry(Gui::Box(Gui::Coord(10, 10), Gui::Size(60, 12)));

    EXPECT_EQ(Gui::Coord(10, 10), pFirst->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Coord(43, 10), pSecond->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Size(30, 12), pFirst->relativeBoundary().size());
}

TEST(GuiLayoutTests, SetGeometry_Spacing_PushesWhatFollowsItAlong)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 0);

    auto* pFirst = itemOfSize(&root, Gui::Size(30, 10));
    auto* pSecond = itemOfSize(&root, Gui::Size(30, 10));
    layout.add(pFirst);
    layout.addSpacing(25);
    layout.add(pSecond);

    layout.setGeometry(Gui::Box(Gui::Coord(0, 0), Gui::Size(30, 45)));

    EXPECT_EQ(0, pFirst->relativeBoundary().minCorner().y());
    EXPECT_EQ(35, pSecond->relativeBoundary().minCorner().y());
    EXPECT_EQ(Gui::Size(30, 45), layout.sizeHint());
}

TEST(GuiLayoutTests, SetGeometry_NestedLayout_PlacesTheInnerEntries)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));

    auto pRow = std::make_unique<GuiBoxLayout>(Direction::HORIZONTAL, 2);
    auto* pLeft = itemOfSize(&root, Gui::Size(20, 10));
    auto* pRight = itemOfSize(&root, Gui::Size(30, 10));
    pRow->add(pLeft);
    pRow->add(pRight);

    GuiBoxLayout column(Direction::VERTICAL, 4);
    auto* pHeading = itemOfSize(&root, Gui::Size(50, 8));
    column.add(pHeading);
    column.add(std::move(pRow));

    // The row is 20 + 2 + 30 wide and 10 tall, under an 8 tall heading.
    EXPECT_EQ(Gui::Size(52, 22), column.sizeHint());

    column.setGeometry(Gui::Box(Gui::Coord(5, 5), Gui::Size(52, 22)));

    EXPECT_EQ(Gui::Coord(5, 5), pHeading->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Coord(5, 17), pLeft->relativeBoundary().minCorner());
    EXPECT_EQ(Gui::Coord(27, 17), pRight->relativeBoundary().minCorner());
}

TEST(GuiLayoutTests, SetGeometry_RunTwice_PutsTheEntriesInTheSamePlace)
{
    GuiTest::Root root(Gui::Box(0, 0, 500, 500));
    GuiBoxLayout layout(Direction::VERTICAL, 5);

    auto* pFirst = itemOfSize(&root, Gui::Size(30, 10));
    auto* pSecond = itemOfSize(&root, Gui::Size(30, 20));
    layout.add(pFirst);
    layout.add(pSecond);

    const Gui::Box box(Gui::Coord(100, 200), Gui::Size(80, 35));
    layout.setGeometry(box);
    const Gui::Box firstPlacement = pSecond->relativeBoundary();
    layout.setGeometry(box);

    EXPECT_EQ(firstPlacement.minCorner(), pSecond->relativeBoundary().minCorner());
    EXPECT_EQ(firstPlacement.size(), pSecond->relativeBoundary().size());
}
