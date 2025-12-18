#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "render/colour.hpp"

TEST(ColourTests, ColourFromString)
{
    struct Case
    {
        std::string_view hex;
        RenColour expected;
    };

    const Case cases[] = {
        { "#000", RenColour::black() },   { "#000000", RenColour::black() }, { "#fff", RenColour::white() },
        { "#FFF", RenColour::white() },   { "#FFFFFF", RenColour::white() }, { "#0FF", RenColour::cyan() },
        { "#0000FF", RenColour::blue() }, { "#00FF00", RenColour::green() }, { "#FF0000", RenColour::red() },
    };

    for (const auto& testCase : cases)
    {
        std::optional<RenColour> colour = RenColour::fromString(testCase.hex);
        EXPECT_TRUE(colour.has_value()) << "Failed to parse " << testCase.hex;
        EXPECT_EQ(testCase.expected, *colour) << "Mismatch for " << testCase.hex;
    }
}
