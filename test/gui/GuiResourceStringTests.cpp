#include <gtest/gtest.h>

#include "gui/ResourceString.hpp"

//////////////////////////////////////////////////////////////////////////////

TEST(GuiResourceStringTests, SubstituteArguments_PlainText_IsCopied)
{
    EXPECT_EQ("Launch", Gui::substituteArguments("Launch", {}));
    EXPECT_EQ("", Gui::substituteArguments("", {}));
    EXPECT_EQ("Launch", Gui::substituteArguments("Launch", { "unused" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_Escapes_AreReplacedInOrder)
{
    EXPECT_EQ("Launch cost 40", Gui::substituteArguments("Launch cost %1", { "40" }));
    EXPECT_EQ("Armor 3/9", Gui::substituteArguments("Armor %1/%2", { "3", "9" }));
    EXPECT_EQ("Save View 1-4", Gui::substituteArguments("Save View %1-%2", { "1", "4" }));
    EXPECT_EQ("red : blue (3 4)", Gui::substituteArguments("%1 : %2 (%3 %4)", { "red", "blue", "3", "4" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_Escapes_MayRepeatAndReorder)
{
    EXPECT_EQ("Bob left, without Bob", Gui::substituteArguments("%1 left, without %1", { "Bob" }));
    EXPECT_EQ("b a", Gui::substituteArguments("%2 %1", { "a", "b" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_DoubledPercent_IsOnePercent)
{
    EXPECT_EQ("Constructed 50%", Gui::substituteArguments("Constructed %1%%", { "50" }));
    EXPECT_EQ("%", Gui::substituteArguments("%%", { }));
    EXPECT_EQ("%1", Gui::substituteArguments("%%1", { "unused" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_ClosingPercent_LetsADigitFollow)
{
    // Without the closing sign the number would read as escape twelve.
    EXPECT_EQ("SW42", Gui::substituteArguments("SW%1%2", { "4" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_MissingArgument_YieldsNothing)
{
    EXPECT_EQ("COST 5 ", Gui::substituteArguments("COST %1 %2", { "5" }));
    EXPECT_EQ("COST  ", Gui::substituteArguments("COST %1 %2", { }));
}

TEST(GuiResourceStringTests, SubstituteArguments_ArgumentContainingAnEscape_IsCopiedVerbatim)
{
    // An argument is text, not a template. Player names reach this function
    // straight off the network, so a name may say anything at all.
    EXPECT_EQ("*System : %1 has lost!", Gui::substituteArguments("*System : %1 has lost!", { "%1" }));
    EXPECT_EQ("*System : a%1 has lost!", Gui::substituteArguments("*System : %1 has lost!", { "a%1" }));
    EXPECT_EQ("*System : a%0 has lost!", Gui::substituteArguments("*System : %1 has lost!", { "a%0" }));
    EXPECT_EQ("100%%", Gui::substituteArguments("%1", { "100%%" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_MalformedText_IsLeftAlone)
{
    // Nothing here names an argument, so nothing is substituted and nothing is lost.
    EXPECT_EQ("50%", Gui::substituteArguments("50%", { "unused" }));
    EXPECT_EQ("%a", Gui::substituteArguments("%a", { "unused" }));
    EXPECT_EQ("%", Gui::substituteArguments("%", { }));
    EXPECT_EQ("% 1", Gui::substituteArguments("% 1", { "unused" }));
}

TEST(GuiResourceStringTests, SubstituteArguments_ZeroAndOutOfRangeNumbers_YieldNothing)
{
    // Numbering starts at one, and a run of digits long enough to overflow a
    // counter must still not name an argument.
    EXPECT_EQ("x", Gui::substituteArguments("%0x", { "a" }));
    EXPECT_EQ("x", Gui::substituteArguments("%99x", { "a" }));
    EXPECT_EQ("x", Gui::substituteArguments("%99999999999999999999999x", { "a" }));
}
