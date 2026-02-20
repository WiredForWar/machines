#include <gtest/gtest.h>
#include "gui/gui.hpp"
#include "gui/RootSharedBitmaps.hpp"

//////////////////////////////////////////////////////////////////////////////

TEST(GuiRootSharedBitmapsTests, NamedBitmap_GetNamedBitmap_RefCount)
{
    GuiRootSharedBitmaps sharedBitmaps;

    sharedBitmaps.createUpdateNamedBitmap("backdrop", "gui/menu/acclaim.bmp", 1);

    {
        auto currentBackdrop = sharedBitmaps.getNamedBitmap("backdrop");
        ASSERT_EQ(2L, currentBackdrop.use_count());
    }

    auto currentBackdrop = sharedBitmaps.getSharedBitmap("gui/menu/acclaim.bmp");
    ASSERT_EQ(1L, currentBackdrop.use_count());
}

TEST(GuiRootSharedBitmapsTests, AllBitmaps_GetBitmaps_NoExist)
{
    GuiRootSharedBitmaps sharedBitmaps;

    auto namedBitmap = sharedBitmaps.getNamedBitmap("i was never created");
    auto sharedBitmap = sharedBitmaps.getSharedBitmap("i was never loaded");

    ASSERT_FALSE(namedBitmap);
    ASSERT_TRUE(sharedBitmap.expired());
}

TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmapFromArea_WhenInvalidBitmap)
{
    GuiRootSharedBitmaps sharedBitmaps;

    // Attempting to blit a never-created bitmap should not crash.
    auto backdrop = sharedBitmaps.getNamedBitmap("backdrop");
    sharedBitmaps.blitNamedBitmapFromArea(backdrop, Gui::Box(), Gui::Coord(), [](const Gui::Box& src) {
        return src;
    });
}
