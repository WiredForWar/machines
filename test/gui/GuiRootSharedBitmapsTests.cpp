#include <gtest/gtest.h>
#include "gui/gui.hpp"
#include "gui/RootSharedBitmaps.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>

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

class GuiRootSharedBitmapsScalingTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        directory_ = std::filesystem::temp_directory_path() / ("machines-bitmap-cache-" + std::to_string(unique));
        ASSERT_TRUE(std::filesystem::create_directory(directory_));
        // RenSurfaceOverrides supplies the surfaces. Only file existence matters
        // here: the test exercises cache identity, not a GPU or image decoder.
        std::ofstream(directory_ / "image.bmp");
        std::ofstream(directory_ / "image_4x.png");
        path_ = (directory_ / "image.bmp").generic_string();
    }

    void TearDown() override
    {
        std::filesystem::remove(directory_ / "image.bmp");
        std::filesystem::remove(directory_ / "image_4x.png");
        std::filesystem::remove(directory_);
    }

    std::filesystem::path directory_;
    std::string path_;
};

TEST_F(GuiRootSharedBitmapsScalingTests, ARepeatedRequestAtTheSameScaleReusesTheBitmap)
{
    GuiRootSharedBitmaps bitmaps;
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 4);
    const auto original = bitmaps.getNamedBitmap("backdrop");
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 4);
    EXPECT_EQ(original, bitmaps.getNamedBitmap("backdrop"));
}

TEST_F(GuiRootSharedBitmapsScalingTests, ANewScaleDoesNotReuseThePreviousSize)
{
    GuiRootSharedBitmaps bitmaps;
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 1);
    const auto original = bitmaps.getNamedBitmap("backdrop");
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 4);
    const auto enlarged = bitmaps.getNamedBitmap("backdrop");
    EXPECT_NE(original, enlarged);
    EXPECT_EQ(enlarged, bitmaps.getSharedBitmap(path_).lock());
}

TEST_F(GuiRootSharedBitmapsScalingTests, ReturningToTheOriginalScaleReplacesTheEnlargedBitmap)
{
    GuiRootSharedBitmaps bitmaps;
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 4);
    const auto enlarged = bitmaps.getNamedBitmap("backdrop");
    bitmaps.createUpdateNamedBitmap("backdrop", path_, 1);
    EXPECT_NE(enlarged, bitmaps.getNamedBitmap("backdrop"));
}
