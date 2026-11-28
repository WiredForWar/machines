#include "machgui/MenuBackdropLayout.hpp"
#include "render/MenuPresentation.hpp"
#include <gtest/gtest.h>
#include <utility>

TEST(MenuPresentationTests, FittedControlsPreserveAspectAndFitBothDimensions)
{
    for (const auto [w, h] : {std::pair{640,480}, {800,600}, {1280,720}, {1920,1080},
             {2560,1440}, {3840,2160}, {3440,1440}, {5120,1440}, {1920,1200}})
    {
        const double scale = MachGui::fittedMenuScale(w, h);
        EXPECT_LE(640 * scale, w + .01);
        EXPECT_LE(480 * scale, h + .01);
        EXPECT_TRUE(std::abs(640 * scale - w) < .01 || std::abs(480 * scale - h) < .01);
    }
    EXPECT_DOUBLE_EQ(MachGui::fittedMenuScale(3840,2160), 4.5);
}

TEST(MenuPresentationTests, EverySlideFrameCoversTheViewportWithoutOutOfBoundsSlices)
{
    for (const auto [w, h] : {std::pair{640,480}, {1920,1080}, {3840,2160}, {3440,1440}})
    {
        const auto placement = Ren::menuBoardPlacement(w, h);
        int previous = 0;
        for (unsigned long frame = 0; frame <= 100; ++frame)
        {
            const int shift = Ren::menuTransitionOffset(frame, 101, w);
            EXPECT_GE(shift, previous);
            EXPECT_LE(shift, w);
            previous = shift;
            int covered = 0;
            for (const int offset : {-shift, w - shift})
            {
                for (const auto slice : {
                    Ren::clippedMenuSlice(0,480,offset,placement.left,w),
                    Ren::clippedMenuSlice(480,2880,offset+placement.left,placement.width,w),
                    Ren::clippedMenuSlice(3360,480,offset+placement.left+placement.width,
                        w-placement.left-placement.width,w)})
                {
                    EXPECT_GE(slice.destX, 0);
                    EXPECT_LE(slice.destX + slice.destWidth, w);
                    if (slice.destWidth > 0)
                    {
                        EXPECT_GT(slice.sourceWidth, 0);
                        EXPECT_GE(slice.sourceX, 0);
                        EXPECT_LE(slice.sourceX + slice.sourceWidth, 3840);
                    }
                    covered += slice.destWidth;
                }
            }
            EXPECT_EQ(covered, w);
        }
        EXPECT_EQ(Ren::menuTransitionOffset(0,101,w), 0);
        EXPECT_EQ(Ren::menuTransitionOffset(100,101,w), w);
    }
}

TEST(MenuPresentationTests, EmptyAndOffscreenSlicesAreNotSubmitted)
{
    EXPECT_EQ(Ren::clippedMenuSlice(0,480,0,0,3840).destWidth, 0);
    EXPECT_EQ(Ren::clippedMenuSlice(0,480,4000,480,3840).destWidth, 0);
    EXPECT_EQ(Ren::clippedMenuSlice(0,480,-480,480,3840).destWidth, 0);
    EXPECT_EQ(Ren::menuTransitionOffset(0,1,3840), 3840);
}
