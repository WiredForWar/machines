#include <gtest/gtest.h>

#include "gui/Displayable.hpp"
#include "gui/Root.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace
{

// A displayable with nothing to draw, so that geometry can be exercised without
// a render device.
class TestDisplayable : public GuiDisplayable
{
public:
    TestDisplayable(GuiDisplayable* pParent, const Gui::Box& box)
        : GuiDisplayable(pParent, box)
    {
    }

    int resizeCount() const { return resizeCount_; }

protected:
    void doDisplay() override { }

    void doResized() override { ++resizeCount_; }

private:
    int resizeCount_{};
};

// Only a root may be built without a parent to place it against.
class TestRoot : public GuiRoot
{
public:
    explicit TestRoot(const Gui::Box& box)
        : GuiRoot(box)
    {
    }

protected:
    void doDisplay() override { }
    void update() override { }
    bool doHandleRightClickEvent(const GuiMouseEvent&) override { return false; }
    void doBecomeRoot() override { }
    void doBecomeNotRoot() override { }
};

} // namespace

//////////////////////////////////////////////////////////////////////////////

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_Move_TakesTheChildrenAlong)
{
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new TestDisplayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new TestDisplayable(pParent, Gui::Box(2, 3, 8, 9));

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
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new TestDisplayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new TestDisplayable(pParent, Gui::Box(2, 2, 20, 20));
    auto* pGrandchild = new TestDisplayable(pChild, Gui::Box(1, 1, 5, 5));

    ASSERT_EQ(13, pGrandchild->absoluteBoundary().minCorner().x());

    pParent->setRelativeBoundary(Gui::Box(110, 10, 150, 50));

    EXPECT_EQ(113, pGrandchild->absoluteBoundary().minCorner().x());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_Resize_ReportsTheNewSize)
{
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new TestDisplayable(&root, Gui::Box(10, 10, 50, 50));

    ASSERT_EQ(0, pItem->resizeCount());

    pItem->setRelativeBoundary(Gui::Box(10, 10, 90, 70));

    EXPECT_EQ(1, pItem->resizeCount());
    EXPECT_EQ(80u, pItem->width());
    EXPECT_EQ(60u, pItem->height());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_MoveOnly_IsNotAResize)
{
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new TestDisplayable(&root, Gui::Box(10, 10, 50, 50));

    pItem->setRelativeBoundary(Gui::Box(20, 20, 60, 60));

    EXPECT_EQ(0, pItem->resizeCount());
}

TEST(GuiDisplayableGeometryTests, SetRelativeBoundary_ResizeInPlace_LeavesTheChildrenWhereTheyWere)
{
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pParent = new TestDisplayable(&root, Gui::Box(10, 10, 50, 50));
    auto* pChild = new TestDisplayable(pParent, Gui::Box(2, 2, 8, 8));

    // The corner the children are placed against has not moved, so a displayable
    // that does not arrange its children has nothing to do.
    pParent->setRelativeBoundary(Gui::Box(10, 10, 90, 90));

    EXPECT_EQ(12, pChild->absoluteBoundary().minCorner().x());
    EXPECT_EQ(12, pChild->absoluteBoundary().minCorner().y());
}

TEST(GuiDisplayableGeometryTests, SizeHint_WhenNothingMeasuresItself_IsTheSizeItWasGiven)
{
    TestRoot root(Gui::Box(0, 0, 200, 200));
    auto* pItem = new TestDisplayable(&root, Gui::Box(10, 10, 50, 40));

    EXPECT_EQ(Gui::Size(40, 30), pItem->sizeHint());
    EXPECT_EQ(pItem->sizeHint(), pItem->minimumSizeHint());

    pItem->setRelativeBoundary(Gui::Box(10, 10, 110, 60));

    EXPECT_EQ(Gui::Size(100, 50), pItem->sizeHint());
}
