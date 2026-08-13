#include <gtest/gtest.h>

#include "gui/gui.hpp"

// The two shapes the menu atlases come in, in the measurements of the unscaled art,
// and what they scale to. A row of the options screen is eleven of the unscaled art.
namespace
{

constexpr int SmallCapHeight = 5;
constexpr int SmallAscender = 6;
constexpr int LargeCapHeight = 14;
constexpr int LargeAscender = 18;
constexpr int RowHeight = 11;

// Where the top of the line box lands, which is where drawing starts.
int textTop(int boxHeight, int capHeight, int ascender)
{
    return Gui::baselineIn(boxHeight, capHeight) - ascender;
}

} // namespace

TEST(GuiBaselineTests, TheCapitalsSitInTheMiddleOfTheBox)
{
    // Five of capital in a box of eleven leaves three either side of it.
    EXPECT_EQ(3 + SmallCapHeight, Gui::baselineIn(RowHeight, SmallCapHeight));

    const int top = textTop(RowHeight, SmallCapHeight, SmallAscender);
    EXPECT_EQ(2, top);

    const int capTop = top + (SmallAscender - SmallCapHeight);
    const int capBottom = top + SmallAscender;
    EXPECT_EQ(capTop, RowHeight - capBottom);
}

TEST(GuiBaselineTests, ScalingTheArtAndTheBoxTogetherScalesTheAnswer)
{
    // The same line, twice the size, lands twice as far down. The 2x atlas is the 1x
    // art scaled whole, so its metrics are the 1x metrics doubled.
    EXPECT_EQ(2, textTop(RowHeight, SmallCapHeight, SmallAscender));
    EXPECT_EQ(4, textTop(2 * RowHeight, 2 * SmallCapHeight, 2 * SmallAscender));
    EXPECT_EQ(6, textTop(3 * RowHeight, 3 * SmallCapHeight, 3 * SmallAscender));
}

TEST(GuiBaselineTests, TheBaselineDoesNotDependOnWhatIsDrawnAroundTheText)
{
    // Which is the point of asking for a baseline rather than working one out: a bare
    // label and a bordered control of the same height land on one line, because
    // neither the border nor the padding is an argument.
    constexpr int Box = 2 * RowHeight;

    EXPECT_EQ(Gui::baselineIn(Box, 2 * SmallCapHeight), Gui::baselineIn(Box, 2 * SmallCapHeight));

    // A capital reaches the same distance up from the baseline in either, so the
    // capitals coincide and not merely the baselines.
    const int top = textTop(Box, 2 * SmallCapHeight, 2 * SmallAscender);
    EXPECT_EQ(6, top + (2 * SmallAscender - 2 * SmallCapHeight));
}

TEST(GuiBaselineTests, ATallerCapitalInTheSameBoxStandsOnALowerBaseline)
{
    // Two fonts are not made to line up by this on their own: each has its capitals
    // centred, so the taller of the two reaches further down as well as further up.
    // Lining those up wants the row to hand both of them one baseline.
    constexpr int Box = 40;

    EXPECT_LT(Gui::baselineIn(Box, SmallCapHeight), Gui::baselineIn(Box, LargeCapHeight));
}

TEST(GuiBaselineTests, CentringTheLineBoxWouldPutTheTextLow)
{
    // What this replaces: centring the whole line box drops the capitals by the room
    // the box keeps above them. At the scale of two that is a pixel, which is what
    // made a label and its drop down disagree.
    const int byBaseline = textTop(2 * RowHeight, 2 * SmallCapHeight, 2 * SmallAscender);
    const int byLineBox = (2 * RowHeight - 2 * SmallAscender) / 2;

    EXPECT_EQ(4, byBaseline);
    EXPECT_EQ(5, byLineBox);
}

TEST(GuiBaselineTests, ABoxNoTallerThanTheCapitalsStartsAtTheTop)
{
    EXPECT_EQ(SmallCapHeight, Gui::baselineIn(SmallCapHeight, SmallCapHeight));
    EXPECT_EQ(LargeCapHeight, Gui::baselineIn(LargeCapHeight, LargeCapHeight));
}
