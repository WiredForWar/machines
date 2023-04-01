#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gui/Action.hpp"
#include "gui/event.hpp"
#include "gui/gui.hpp"
#include "private/shrkeybd.hpp"

//////////////////////////////////////////////////////////////////////////////

TEST(GuiActionTests, MatchKeyEventToShortcut)
{
    GuiKeySequence keySequence("A");
    GuiKeyEvent event(GuiKey(DevKey::KEY_A), Gui::ButtonState::PRESSED, 0, DevButtonEvent());

    EXPECT_TRUE(keySequence.matches(event));
}

// TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmapFromArea_WhenInvalidBitmap)
//{
//     MockPainter mockPainter;

//    EXPECT_CALL(mockPainter, blit(An<const GuiBitmap&>(), An<const Gui::Box&>(), An<const Gui::Coord&>())).Times(0);

//    auto sharedBitmaps = GuiRootSharedBitmaps { mockPainter };

//    auto backdrop = sharedBitmaps.getNamedBitmap("backdrop");
//    sharedBitmaps.blitNamedBitmapFromArea(backdrop, Gui::Box(), Gui::Coord(), [](const Gui::Box& src) { return src;
//    });
//}
