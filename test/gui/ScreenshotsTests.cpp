#include "gui/Screenshots.hpp"

#include <gtest/gtest.h>

#include <string>

namespace
{

// A name is only worth reporting on if it is bad, so the tests read better
// asking the question this way round.
bool isAccepted(const std::string& fileName)
{
    return !Gui::screenshotNameComplaint(fileName).has_value();
}

} // namespace

TEST(ScreenshotsTests, ADescriptiveNameIsAccepted)
{
    EXPECT_TRUE(isAccepted("desert"));
    EXPECT_TRUE(isAccepted("m_desert_zenith_01"));
    EXPECT_TRUE(isAccepted("m_desert.zenith.png"));
    EXPECT_TRUE(isAccepted("0"));
}

TEST(ScreenshotsTests, ANameThatCouldReachAnotherDirectoryIsRefused)
{
    EXPECT_FALSE(isAccepted("shots/desert"));
    EXPECT_FALSE(isAccepted("shots\\desert"));
    EXPECT_FALSE(isAccepted(".."));
    EXPECT_FALSE(isAccepted("../desert"));
    EXPECT_FALSE(isAccepted("/etc/passwd"));
    EXPECT_FALSE(isAccepted("C:\\windows\\desert"));
}

TEST(ScreenshotsTests, ANameWithNothingUsableInItIsRefused)
{
    EXPECT_FALSE(isAccepted(""));
    EXPECT_FALSE(isAccepted(" "));
    EXPECT_FALSE(isAccepted("des ert"));
    EXPECT_FALSE(isAccepted("desert!"));
    EXPECT_FALSE(isAccepted("desert*"));
    EXPECT_FALSE(isAccepted(std::string(65, 'a')));
}

// A leading dot names a hidden file on one platform and nothing useful on the
// other, and a leading dash reads as an option to whatever is handed the name.
TEST(ScreenshotsTests, ANameStartsWithALetterOrADigit)
{
    EXPECT_FALSE(isAccepted(".desert"));
    EXPECT_FALSE(isAccepted("_desert"));
    EXPECT_FALSE(isAccepted("-desert"));
}

TEST(ScreenshotsTests, EveryComplaintNamesTheOffendingName)
{
    const std::string offender = "no good/at all";

    const std::optional<std::string> complaint = Gui::screenshotNameComplaint(offender);
    ASSERT_TRUE(complaint.has_value());
    EXPECT_NE(complaint->find(offender), std::string::npos) << complaint.value();
}

TEST(ScreenshotsTests, ANameGetsTheExtensionItIsMissing)
{
    EXPECT_EQ(Gui::screenshotPath("desert").pathname(), "screenshots/desert.png");
    EXPECT_EQ(Gui::screenshotPath("desert.png").pathname(), "screenshots/desert.png");
    EXPECT_EQ(Gui::screenshotPath("desert.PNG").pathname(), "screenshots/desert.PNG");
    EXPECT_EQ(Gui::screenshotPath("desert.zenith").pathname(), "screenshots/desert.zenith.png");
}
