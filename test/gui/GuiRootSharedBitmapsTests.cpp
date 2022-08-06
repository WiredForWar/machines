#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "gui/border.hpp"
#include "gui/painter.hpp"
#include "gui/RootSharedBitmaps.hpp"

using ::testing::An;
using ::testing::TypedEq;

class MockPainter : public IGuiPainter
{
public:
    virtual ~MockPainter() = default;

    MOCK_METHOD(void, blit, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest), (const, override));
    MOCK_METHOD(void, blit, (const GuiBitmap& source, const Gui::Coord& dest), (const, override));
    MOCK_METHOD(void, blit, (const GuiBitmap& source), (const, override));
    MOCK_METHOD(void, tile, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea), (const, override));
    MOCK_METHOD(void, stretch, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea), (const, override));
    MOCK_METHOD(void, stretch, (const GuiBitmap& source, const Gui::Box& destArea ), (const, override));
    MOCK_METHOD(void, filledRectangle, ( const Gui::Box&, const Gui::Colour& ), (const, override));
    MOCK_METHOD(void, hollowRectangle, ( const Gui::Box&, const Gui::Colour&, unsigned thickness ), (const, override));
    MOCK_METHOD(void, bevel, ( const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol), (const, override));
    MOCK_METHOD(void, line, ( const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour&, unsigned thickness), (const, override));
    MOCK_METHOD(void, horizontalLine, ( const Gui::Coord& c1, unsigned length, const Gui::Colour&, unsigned thickness), (const, override));
    MOCK_METHOD(void, verticalLine, ( const Gui::Coord& c1, unsigned height, const Gui::Colour&, unsigned thickness), (const, override));
    MOCK_METHOD(void, text, ( const Gui::Coord& c, const string& text, const Gui::Colour&), (const, override));
    MOCK_METHOD(void, rightAlignText, (const Gui::Coord& c, const string& theText, const Gui::Colour& col), (const, override));
    MOCK_METHOD(void, filledBorder, ( const Gui::Coord& absCoord, const GuiBorderDimensions&, const GuiFilledBorderColours&, const GuiBorderMetrics&), (const, override));
};

//////////////////////////////////////////////////////////////////////////////

TEST(GuiRootSharedBitmapsTests, NamedBitmap_GetNamedBitmap_RefCount)
{
    MockPainter mockPainter;

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };

    sharedBitmaps.createUpdateNamedBitmap("backdrop", "gui/menu/acclaim.bmp");

    {
        auto currentBackdrop = sharedBitmaps.getNamedBitmap("backdrop");
        ASSERT_EQ(2L, currentBackdrop.use_count());
    }

    auto currentBackdrop = sharedBitmaps.getSharedBitmap("gui/menu/acclaim.bmp");
    ASSERT_EQ(1L, currentBackdrop.use_count());
}

TEST(GuiRootSharedBitmapsTests, AllBitmaps_GetBitmaps_NoExist)
{
    MockPainter mockPainter;

    auto sharedBitmaps = GuiRootSharedBitmaps{mockPainter};

    auto namedBitmap = sharedBitmaps.getNamedBitmap("i was never created");
    auto sharedBitmap = sharedBitmaps.getSharedBitmap("i was never loaded");

    ASSERT_FALSE(namedBitmap);
    ASSERT_TRUE(sharedBitmap.expired());
}

TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmapFromArea)
{
    MockPainter mockPainter;

    auto srcBox   = Gui::Box{ Gui::Coord{25, 25}, 1.0 };
    auto dstPoint = Gui::Coord{ 0, 0 };

    EXPECT_CALL(mockPainter, blit(An<const GuiBitmap&>(), TypedEq<const Gui::Box&>(srcBox), TypedEq<const Gui::Coord&>(dstPoint)))
        .Times(1);

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };
    sharedBitmaps.createUpdateNamedBitmap("backdrop", "gui/menu/acclaim.bmp");

    auto backdrop = sharedBitmaps.getNamedBitmap("backdrop");
    sharedBitmaps.blitNamedBitmapFromArea(backdrop, srcBox, dstPoint, [](auto& src){
        return Gui::Box(
                    Gui::Coord( src.minCorner().x() - 0, src.minCorner().y() - 0 ),
                    src.maxCorner().x() - src.minCorner().x(),
                    src.maxCorner().y() - src.minCorner().y()
                );
    });
}

TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmapFromArea_WhenInvalidBitmap)
{
    MockPainter mockPainter;

    EXPECT_CALL(mockPainter, blit(An<const GuiBitmap&>(), An<const Gui::Box&>(), An<const Gui::Coord&>()))
            .Times(0);

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };

    auto backdrop = sharedBitmaps.getNamedBitmap("backdrop");
    sharedBitmaps.blitNamedBitmapFromArea(backdrop, Gui::Box(), Gui::Coord(), [](auto& src){
        return src;
    });
}

TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmap)
{
    MockPainter mockPainter;

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };

    //TODO: Write test
    ASSERT_TRUE(false);
}
