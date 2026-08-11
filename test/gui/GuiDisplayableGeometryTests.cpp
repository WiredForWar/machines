#include <gtest/gtest.h>

#include "TestDisplayable.hpp"

//////////////////////////////////////////////////////////////////////////////

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_Move_TakesTheChildrenAlong)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new GuiTest::Displayable(pParent, Gui::Box(2, 3, 8, 9));

    ASSERT_EQ(12, pChild->absoluteBoundary().minCorner().x());
    ASSERT_EQ(13, pChild->absoluteBoundary().minCorner().y());

    // Same size, moved by (20, 5).
    pParent->setRelativeBoundary(Gui::Box(30, 15, 70, 55));

    EXPECT_EQ(32, pChild->absoluteBoundary().minCorner().x());
    EXPECT_EQ(18, pChild->absoluteBoundary().minCorner().y());

    // The child did not move within its parent.
    EXPECT_EQ(2, pChild->relativeBoundary().minCorner().x());
    EXPECT_EQ(3, pChild->relativeBoundary().minCorner().y());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_Move_ReachesAGrandchild)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new GuiTest::Displayable(pParent, Gui::Box(2, 2, 20, 20));
    auto* pGrandchild = new GuiTest::Displayable(pChild, Gui::Box(1, 1, 5, 5));

    ASSERT_EQ(13, pGrandchild->absoluteBoundary().minCorner().x());

    pParent->setRelativeBoundary(Gui::Box(110, 10, 150, 50));

    EXPECT_EQ(113, pGrandchild->absoluteBoundary().minCorner().x());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_Resize_ReportsTheNewSize)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 50));

    ASSERT_EQ(0, pItem->resizeCount());

    pItem->setRelativeBoundary(Gui::Box(10, 10, 90, 70));

    EXPECT_EQ(1, pItem->resizeCount());
    EXPECT_EQ(80u, pItem->width());
    EXPECT_EQ(60u, pItem->height());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_MoveOnly_IsNotAResize)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 50));

    pItem->setRelativeBoundary(Gui::Box(20, 20, 60, 60));

    EXPECT_EQ(0, pItem->resizeCount());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_ResizeInPlace_LeavesTheChildrenWhereTheyWere)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new GuiTest::Displayable(pParent, Gui::Box(2, 2, 8, 8));

    // The corner the children are placed against has not moved, so a displayable
    // that does not arrange its children has nothing to do.
    pParent->setRelativeBoundary(Gui::Box(10, 10, 90, 90));

    EXPECT_EQ(12, pChild->absoluteBoundary().minCorner().x());
    EXPECT_EQ(12, pChild->absoluteBoundary().minCorner().y());
}

TEST(GuiDisplayableGeometryTests, SizeHint_WhenNothingMeasuresItself_IsTheSizeItWasGiven)
{
    GuiTest::Root root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(10, 10, 50, 40));

    EXPECT_EQ(Gui::Size(40, 30), pItem->sizeHint());
    EXPECT_EQ(pItem->sizeHint(), pItem->minimumSizeHint());

    pItem->setRelativeBoundary(Gui::Box(10, 10, 110, 60));

    EXPECT_EQ(Gui::Size(100, 50), pItem->sizeHint());
}
