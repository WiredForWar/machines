#include "render/TextWrap.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

namespace
{

// One unit per character, so that a width in the tests reads as a character count
// and the cases stay about where the break lands rather than about font metrics.
int monospace(std::string_view text)
{
    return static_cast<int>(text.size());
}

std::vector<std::string> wrap(std::string_view text, int maxWidth)
{
    const std::vector<std::string_view> lines = Ren::wrapText(text, maxWidth, monospace);
    return std::vector<std::string>(lines.begin(), lines.end());
}

} // namespace

TEST(TextWrapTests, ATextThatFitsIsLeftAlone)
{
    EXPECT_EQ(wrap("hello", 10), (std::vector<std::string>{ "hello" }));
}

TEST(TextWrapTests, ATextThatExactlyFillsTheWidthIsLeftAlone)
{
    EXPECT_EQ(wrap("0123456789", 10), (std::vector<std::string>{ "0123456789" }));
}

TEST(TextWrapTests, AnEmptyTextIsOneEmptyLine)
{
    EXPECT_EQ(wrap("", 10), (std::vector<std::string>{ "" }));
}

TEST(TextWrapTests, ABreakTakesTheLastSpaceThatFits)
{
    // "hello worl" is as much as fits; the space at 5 is the last one within it.
    EXPECT_EQ(wrap("hello world foo", 10), (std::vector<std::string>{ "hello", "world foo" }));
}

TEST(TextWrapTests, TheSpaceBrokenAtIsNotKeptOnEitherLine)
{
    const std::vector<std::string> lines = wrap("hello world foo", 10);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0].back(), 'o');
    EXPECT_EQ(lines[1].front(), 'w');
}

TEST(TextWrapTests, AWordWithNoSpaceBreaksWhereItStopsFitting)
{
    EXPECT_EQ(
        wrap("abcdefghijklmnopqrstuvwxyz", 10),
        (std::vector<std::string>{ "abcdefghij", "klmnopqrst", "uvwxyz" }));
}

TEST(TextWrapTests, ASpaceSixteenCharactersBackIsStillUsed)
{
    // Width 20, so the break falls after index 19. The space sits at index 4,
    // which is the sixteenth character looked back over.
    const std::string text = "wide " + std::string(30, 'c');

    const std::vector<std::string> lines = wrap(text, 20);
    ASSERT_GE(lines.size(), 2u);
    EXPECT_EQ(lines[0], "wide");
    EXPECT_EQ(lines[1], std::string(20, 'c'));
}

TEST(TextWrapTests, ASpaceFurtherBackThanSixteenCharactersIsIgnored)
{
    // The space is seventeen characters back from the break, so the long word is
    // cut where it stops fitting instead.
    const std::string text = "no " + std::string(30, 'c');

    const std::vector<std::string> lines = wrap(text, 20);
    ASSERT_GE(lines.size(), 2u);
    EXPECT_EQ(lines[0], "no " + std::string(17, 'c'));
}

TEST(TextWrapTests, ALineIsNeverEmptyWhenThereIsTextLeft)
{
    // A width nothing fits in must still make progress, one character at a time,
    // rather than producing empty lines for ever.
    EXPECT_EQ(wrap("abc", 0), (std::vector<std::string>{ "a", "b", "c" }));
}

TEST(TextWrapTests, ALeadingSpaceIsNotBrokenAtIntoAnEmptyLine)
{
    const std::vector<std::string> lines = wrap(" abcdefghijkl", 10);
    ASSERT_FALSE(lines.empty());
    EXPECT_FALSE(lines[0].empty());
}
