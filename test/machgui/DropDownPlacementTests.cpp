#include <gtest/gtest.h>

#include "machgui/DropDownPlacement.hpp"

namespace
{

// A 10 pixel item with the 9 pixel spacing that drawing items overlapping by one
// pixel gives.
constexpr int ItemHeight = 10;
constexpr int ItemSpacing = 9;

// The bounds a drop down has to stay inside.
Gui::Box screen(int height)
{
    return Gui::Box(0, 0, 200, height);
}

// A closed drop down control of the usual width, with its top at y.
Gui::Box anchorAt(int y)
{
    return Gui::Box(Gui::Coord(20, y), 100, ItemHeight);
}

// The height a run of n items needs: a step of spacing between each neighbouring
// pair, plus the height of the last one.
int heightFor(int items)
{
    return (items - 1) * ItemSpacing + ItemHeight;
}

int topOf(const Gui::Box& box)
{
    return static_cast<int>(box.minCorner().y());
}

int bottomOf(const Gui::Box& box)
{
    return static_cast<int>(box.maxCorner().y());
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DropDownPlacementTests, AListThatFitsOpensDownwardFromTheAnchor)
{
    const auto placement = MachGui::dropDownPlacement(anchorAt(50), screen(480), ItemHeight, ItemSpacing, 4);

    EXPECT_EQ(4u, placement.visibleItems);
    EXPECT_EQ(50, topOf(placement.box));
    EXPECT_EQ(heightFor(4), bottomOf(placement.box) - topOf(placement.box));
}

TEST(DropDownPlacementTests, TheListKeepsTheWidthAndTheLeftEdgeOfTheAnchor)
{
    const auto placement = MachGui::dropDownPlacement(anchorAt(50), screen(480), ItemHeight, ItemSpacing, 4);

    EXPECT_EQ(20, static_cast<int>(placement.box.minCorner().x()));
    EXPECT_EQ(120, static_cast<int>(placement.box.maxCorner().x()));
}

TEST(DropDownPlacementTests, AListTooTallForEitherSideOpensWhereThereIsMoreRoom)
{
    // Anchored near the bottom, a hundred items fit on neither side, so the list
    // takes the roomier one: upward, ending at the anchor's bottom edge.
    const auto anchor = anchorAt(440);
    const auto placement = MachGui::dropDownPlacement(anchor, screen(480), ItemHeight, ItemSpacing, 100);

    EXPECT_EQ(bottomOf(anchor), bottomOf(placement.box));
    EXPECT_GE(topOf(placement.box), 0);
    EXPECT_EQ(49u, placement.visibleItems); // 48 * 9 + 10 == 442, and 450 is there
}

TEST(DropDownPlacementTests, ALongListNeverReachesPastItsBounds)
{
    // A long list opened from anywhere on the screen stays inside its bounds.
    for (int anchorY = 0; anchorY <= 470; anchorY += 10)
    {
        const auto placement = MachGui::dropDownPlacement(anchorAt(anchorY), screen(480), ItemHeight, ItemSpacing, 60);

        EXPECT_GE(topOf(placement.box), 0) << "opened at y=" << anchorY;
        EXPECT_LE(bottomOf(placement.box), 480) << "opened at y=" << anchorY;
        EXPECT_GT(placement.visibleItems, 0u) << "opened at y=" << anchorY;
    }
}

TEST(DropDownPlacementTests, AListWithTooLittleRoomBelowOpensUpwardInFull)
{
    // Two items' worth of room below and plenty above: the list grows up from the
    // anchor's bottom rather than being cut down to two entries.
    const auto anchor = anchorAt(460);
    const auto placement = MachGui::dropDownPlacement(anchor, screen(480), ItemHeight, ItemSpacing, 6);

    EXPECT_EQ(6u, placement.visibleItems);
    EXPECT_EQ(bottomOf(anchor), bottomOf(placement.box));
    EXPECT_EQ(heightFor(6), bottomOf(placement.box) - topOf(placement.box));
}

TEST(DropDownPlacementTests, AListOpeningUpwardIsAlsoCutToWhatFits)
{
    // 60 pixels of screen, anchored at 50: only one item fits below, and the 60
    // above the anchor's bottom hold six.
    const auto placement = MachGui::dropDownPlacement(anchorAt(50), screen(60), ItemHeight, ItemSpacing, 20);

    EXPECT_EQ(6u, placement.visibleItems);
    EXPECT_GE(topOf(placement.box), 0);
    EXPECT_LE(bottomOf(placement.box), 60);
}

TEST(DropDownPlacementTests, AnEmptyListAsksForNothing)
{
    const auto placement = MachGui::dropDownPlacement(anchorAt(50), screen(480), ItemHeight, ItemSpacing, 0);

    EXPECT_EQ(0u, placement.visibleItems);
}

TEST(DropDownPlacementTests, BoundsWithNoRoomAtAllAskForNothing)
{
    // Bounds that cannot hold a single item produce no list at all, rather than a
    // box reaching outside them.
    const auto placement = MachGui::dropDownPlacement(anchorAt(0), Gui::Box(0, 0, 200, 0), ItemHeight, ItemSpacing, 10);

    EXPECT_EQ(0u, placement.visibleItems);
}
