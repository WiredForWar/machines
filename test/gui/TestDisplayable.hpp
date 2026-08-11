#pragma once

#include "gui/Displayable.hpp"
#include "gui/Root.hpp"

// Displayables with nothing to draw. Geometry needs no render device, so a tree
// of these can be built and measured in a test as long as doDisplay() is never
// reached.

namespace GuiTest
{

class Displayable : public GuiDisplayable
{
public:
    Displayable(GuiDisplayable* pParent, const Gui::Box& box)
        : GuiDisplayable(pParent, box)
    {
    }

    int resizeCount() const { return resizeCount_; }

    // A size of its own to report, whatever box it was built with.
    void setSizeHint(const Gui::Size& size) { sizeHint_ = size; }
    void setMinimumSizeHint(const Gui::Size& size) { minimumSizeHint_ = size; }

    Gui::Size sizeHint() const override { return sizeHint_ ? *sizeHint_ : GuiDisplayable::sizeHint(); }

    Gui::Size minimumSizeHint() const override
    {
        return minimumSizeHint_ ? *minimumSizeHint_ : sizeHint();
    }

protected:
    void doDisplay() override { }

    void doResized() override { ++resizeCount_; }

private:
    int resizeCount_{};
    std::optional<Gui::Size> sizeHint_{};
    std::optional<Gui::Size> minimumSizeHint_{};
};

// Only a root may be built without a parent to place it against.
class Root : public GuiRoot
{
public:
    explicit Root(const Gui::Box& box)
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

} // namespace GuiTest
