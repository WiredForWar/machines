#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "gui/border.hpp"
#include "gui/painter.hpp"
#include "gui/RootSharedBitmaps.hpp"

class MockPainter : public IGuiPainter
{
public:
    virtual ~MockPainter() = default;

    MOCK_METHOD(void, blit, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Coord& dest), (override));
    MOCK_METHOD(void, blit, (const GuiBitmap& source, const Gui::Coord& dest), (override));
    MOCK_METHOD(void, blit, (const GuiBitmap& source), (override));
    MOCK_METHOD(void, tile, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea), (override));
    MOCK_METHOD(void, stretch, (const GuiBitmap& source, const Gui::Box& sourceArea, const Gui::Box& destArea), (override));
    MOCK_METHOD(void, stretch, (const GuiBitmap& source, const Gui::Box& destArea ), (override));
    MOCK_METHOD(void, filledRectangle, ( const Gui::Box&, const Gui::Colour& ), (override));
    MOCK_METHOD(void, hollowRectangle, ( const Gui::Box&, const Gui::Colour&, unsigned thickness ), (override));
    MOCK_METHOD(void, bevel, ( const Gui::Box& b, unsigned thickness, const Gui::Colour& hiCol, const Gui::Colour& loCol), (override));
    MOCK_METHOD(void, line, ( const Gui::Coord& c1, const Gui::Coord& c2, const Gui::Colour&, unsigned thickness), (override));
    MOCK_METHOD(void, horizontalLine, ( const Gui::Coord& c1, unsigned length, const Gui::Colour&, unsigned thickness), (override));
    MOCK_METHOD(void, verticalLine, ( const Gui::Coord& c1, unsigned height, const Gui::Colour&, unsigned thickness), (override));
    MOCK_METHOD(void, text, ( const Gui::Coord& c, const string& text, const Gui::Colour&), (override));
    MOCK_METHOD(void, rightAlignText, (const Gui::Coord& c, const string& theText, const Gui::Colour& col), (override));
    MOCK_METHOD(void, filledBorder, ( const Gui::Coord& absCoord, const GuiBorderDimensions&, const GuiFilledBorderColours&, const GuiBorderMetrics&), (override));
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

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };

    //TODO: Write test
    ASSERT_TRUE(false);
}

TEST(GuiRootSharedBitmapsTests, Blitting_blitNamedBitmap)
{
    MockPainter mockPainter;

    auto sharedBitmaps = GuiRootSharedBitmaps{ mockPainter };

    //TODO: Write test
    ASSERT_TRUE(false);
}
