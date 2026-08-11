#ifndef _GUI_LAYOUT_HPP
#define _GUI_LAYOUT_HPP

#include "ctl/Vector.hpp"
#include "gui/gui.hpp"

#include <memory>

class GuiDisplayable;

//////////////////////////////////////////////////////////////////////

// Decides where the contents of a box go.
//
// A layout never owns a displayable. A displayable belongs to its parent and is
// destroyed with it; a layout only says where it sits, so the same displayable
// may be built, placed and destroyed without the layout being involved.

class GuiLayout
// cannonical form revoked
{
public:
    virtual ~GuiLayout();

    // The size the contents would like, and the smallest they fit in.
    virtual Gui::Size sizeHint() const = 0;
    virtual Gui::Size minimumSizeHint() const = 0;

    // Place the contents within box, given in the coordinates the contents are
    // placed in, which is to say relative to their parent.
    virtual void setGeometry(const Gui::Box& box) = 0;

protected:
    GuiLayout();

private:
    GuiLayout(const GuiLayout&) = delete;
    bool operator==(const GuiLayout&) const = delete;
    GuiLayout& operator=(const GuiLayout&) = delete;
};

//////////////////////////////////////////////////////////////////////

// Lays its contents out in one line, either down or across, each one filling the
// box in the other direction. Consecutive entries are separated by the spacing.

class GuiBoxLayout : public GuiLayout
// cannonical form revoked
{
public:
    enum class Direction
    {
        VERTICAL,
        HORIZONTAL,
    };

    explicit GuiBoxLayout(Direction direction, std::size_t spacing = 0);
    ~GuiBoxLayout() override;

    Direction direction() const;

    std::size_t spacing() const;
    void setSpacing(std::size_t spacing);

    // The displayable keeps belonging to its parent. It must outlive this layout,
    // or be dropped from it first.
    void add(GuiDisplayable* pDisplayable);
    // PRE( pDisplayable != nullptr );

    void add(std::unique_ptr<GuiLayout> pLayout);
    // PRE( pLayout != nullptr );

    // A gap of its own, on top of the spacing on either side of it.
    void addSpacing(std::size_t length);

    std::size_t count() const;

    Gui::Size sizeHint() const override;
    Gui::Size minimumSizeHint() const override;
    void setGeometry(const Gui::Box& box) override;

private:
    GuiBoxLayout(const GuiBoxLayout&) = delete;
    bool operator==(const GuiBoxLayout&) const = delete;
    GuiBoxLayout& operator=(const GuiBoxLayout&) = delete;

    // Exactly one of the three says what the entry is.
    struct Entry
    {
        GuiDisplayable* pDisplayable{};
        std::unique_ptr<GuiLayout> pLayout{};
        std::size_t length{};
    };

    using Entries = std::vector<Entry>;

    enum class Wanted
    {
        PREFERRED,
        MINIMUM,
    };

    static Gui::Size entrySize(const Entry& entry, Wanted wanted);
    Gui::Size totalSize(Wanted wanted) const;

    // The extent of size along and across this layout's direction.
    MATHEX_SCALAR along(const Gui::Size& size) const;
    MATHEX_SCALAR across(const Gui::Size& size) const;
    Gui::Size sized(MATHEX_SCALAR alongExtent, MATHEX_SCALAR acrossExtent) const;

    Direction direction_{};
    std::size_t spacing_{};
    Entries entries_{};
};

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _GUI_LAYOUT_HPP
