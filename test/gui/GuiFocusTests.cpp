#include <gtest/gtest.h>

#include "TestDisplayable.hpp"

#include "gui/Manager.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace
{

// A displayable with a reason of its own to decline the keys, standing in for a
// list box with nothing in it.
class PickyDisplayable : public GuiTest::Displayable
{
public:
    PickyDisplayable(GuiDisplayable* pParent, const Gui::Box& box)
        : GuiTest::Displayable(pParent, box)
    {
    }

    void setHasSomethingToDo(bool value) { hasSomethingToDo_ = value; }

    bool isFocusEnabled() const override
    {
        return GuiDisplayable::isFocusEnabled() && hasSomethingToDo_;
    }

private:
    bool hasSomethingToDo_{ true };
};

} // namespace

//////////////////////////////////////////////////////////////////////////////

TEST(GuiFocusTests, CanTakeFocus_WhenItAcceptedTheFocusAndIsEnabled_IsTrue)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(0, 0, 10, 10));
    pItem->setAcceptsFocus(true);

    EXPECT_TRUE(GuiManager::instance().canTakeFocus(pItem));
}

TEST(GuiFocusTests, CanTakeFocus_WhenItNeverAcceptedTheFocus_IsFalse)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(0, 0, 10, 10));

    // An explicit navigation target may name a displayable that does not take keys.
    EXPECT_FALSE(GuiManager::instance().canTakeFocus(pItem));
}

TEST(GuiFocusTests, CanTakeFocus_WhenDisabled_IsFalse)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(0, 0, 10, 10));
    pItem->setAcceptsFocus(true);

    pItem->setEnabled(false);

    // Being disabled keeps the keys away without each displayable saying so.
    EXPECT_FALSE(GuiManager::instance().canTakeFocus(pItem));

    pItem->setEnabled(true);
    EXPECT_TRUE(GuiManager::instance().canTakeFocus(pItem));
}

TEST(GuiFocusTests, CanTakeFocus_WhileFocusIsSuppressed_IsFalse)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    auto* pItem = new GuiTest::Displayable(&root, Gui::Box(0, 0, 10, 10));
    pItem->setAcceptsFocus(true);

    pItem->suppressFocus(true);

    EXPECT_FALSE(GuiManager::instance().canTakeFocus(pItem));
}

TEST(GuiFocusTests, CanTakeFocus_WhenTheDisplayableItselfDeclines_IsFalse)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    auto* pItem = new PickyDisplayable(&root, Gui::Box(0, 0, 10, 10));
    pItem->setAcceptsFocus(true);

    ASSERT_TRUE(GuiManager::instance().canTakeFocus(pItem));

    // What it holds, rather than whether it is switched on, is its own to answer.
    pItem->setHasSomethingToDo(false);

    EXPECT_FALSE(GuiManager::instance().canTakeFocus(pItem));
}

TEST(GuiFocusTests, FocusChain_IsJoinedByAcceptingAndLeftOnDestruction)
{
    GuiTest::Root root(Gui::Box(0, 0, 100, 100));
    ASSERT_TRUE(GuiManager::instance().focusChain().empty());

    {
        auto* pFirst = new GuiTest::Displayable(&root, Gui::Box(0, 0, 10, 10));
        auto* pSecond = new GuiTest::Displayable(&root, Gui::Box(0, 20, 10, 30));
        pFirst->setAcceptsFocus(true);
        pSecond->setAcceptsFocus(true);

        const GuiManager::FocusChain& chain = GuiManager::instance().focusChain();
        ASSERT_EQ(2u, chain.size());

        // In the order they accepted it, and the first to join holds the focus.
        EXPECT_EQ(pFirst, chain.front());
        EXPECT_EQ(pSecond, chain.back());
        EXPECT_TRUE(pFirst->hasFocusSet());
        EXPECT_FALSE(pSecond->hasFocusSet());

        root.deleteAllChildren();
    }

    EXPECT_TRUE(GuiManager::instance().focusChain().empty());
}
