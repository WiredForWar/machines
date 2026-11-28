#include "gui/Displayable.hpp"
#include "gui/Root.hpp"
#include <gtest/gtest.h>

namespace
{
class LayoutNode final : public GuiDisplayable
{
public:
    LayoutNode(GuiDisplayable* parent, const Gui::Box& box) : GuiDisplayable(parent, box) {}
    void doDisplay() override {}
};
class LayoutRoot final : public GuiRoot
{
public:
    explicit LayoutRoot(const Gui::Box& box) : GuiRoot(box) {}
    void update() override {}
    bool doHandleRightClickEvent(const GuiMouseEvent&) override { return false; }
    void doDisplay() override {}
    void doBecomeRoot() override {}
    void doBecomeNotRoot() override {}
};
}

TEST(GuiFractionalLayoutTests, CenteringMenusPreservesFractionalChildExtents)
{
    for (const double scale : {1.0, 1.5, 2.25, 3.0, 4.0, 4.5})
    {
        LayoutRoot root(Gui::Box(0,0,3840,2160));
        const auto box = Gui::Box(425,406,576,439) * scale;
        auto* button = new LayoutNode(&root, box);
        auto* nested = new LayoutNode(button, Gui::Box(1,1,10,12));
        const auto size = button->absoluteBoundary().size();
        const auto nestedSize = nested->absoluteBoundary().size();
        for (const Gui::Coord offset : {Gui::Coord(480,0), Gui::Coord(0,0), Gui::Coord(17.25,3.5)})
        {
            root.absoluteCoord(offset);
            EXPECT_EQ(button->absoluteBoundary(), Gui::Box(box.minCorner() + Gui::Vec(offset.x(),offset.y()), size));
            EXPECT_EQ(button->relativeBoundary(), box);
            EXPECT_EQ(nested->absoluteBoundary().size(), nestedSize);
        }
        button->absoluteCoord({500.25,700.5});
        EXPECT_EQ(button->absoluteBoundary(), Gui::Box(Gui::Coord(500.25,700.5), size));
    }
}
