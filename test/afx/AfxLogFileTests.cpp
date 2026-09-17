#include "afx/AfxLogFile.hpp"

#include <gtest/gtest.h>

TEST(AfxLogFileTests, APlainNameGoesStraightUnderLogs)
{
    const std::optional<std::string> path = afxLogFilePath("machines.txt");

    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, "logs/machines.txt");
}

TEST(AfxLogFileTests, ASubdirectoryIsKept)
{
    const std::optional<std::string> path = afxLogFilePath("events/20260917-204510.log");

    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, "logs/events/20260917-204510.log");
}

TEST(AfxLogFileTests, BackslashesAreSeparatorsToo)
{
    const std::optional<std::string> path = afxLogFilePath("events\\run3.log");

    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, "logs/events/run3.log");
}

TEST(AfxLogFileTests, ANameMayCarryDots)
{
    const std::optional<std::string> path = afxLogFilePath("events/run..3.log");

    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, "logs/events/run..3.log");
}

TEST(AfxLogFileTests, AnEmptyNameIsRefused)
{
    EXPECT_FALSE(afxLogFilePath("").has_value());
}

TEST(AfxLogFileTests, AnAbsolutePathIsRefused)
{
    EXPECT_FALSE(afxLogFilePath("/tmp/machines.log").has_value());
    EXPECT_FALSE(afxLogFilePath("\\machines.log").has_value());
}

TEST(AfxLogFileTests, APathAgainstAnotherDriveIsRefused)
{
    EXPECT_FALSE(afxLogFilePath("c:\\machines.log").has_value());
    EXPECT_FALSE(afxLogFilePath("c:machines.log").has_value());
}

TEST(AfxLogFileTests, APathClimbingOutOfLogsIsRefused)
{
    EXPECT_FALSE(afxLogFilePath("..").has_value());
    EXPECT_FALSE(afxLogFilePath("../machines.log").has_value());
    EXPECT_FALSE(afxLogFilePath("events/../../machines.log").has_value());
    EXPECT_FALSE(afxLogFilePath("events\\..\\..\\machines.log").has_value());
}

TEST(AfxLogFileTests, ADirectoryIsRefused)
{
    EXPECT_FALSE(afxLogFilePath("events/").has_value());
    EXPECT_FALSE(afxLogFilePath("events\\").has_value());
}

TEST(AfxLogFileTests, ARefusalSaysWhy)
{
    std::string whyNot;

    EXPECT_FALSE(afxLogFilePath("../machines.log", &whyNot).has_value());
    EXPECT_FALSE(whyNot.empty());
}

TEST(AfxLogFileTests, AnAcceptedNameLeavesTheReasonAlone)
{
    std::string whyNot = "untouched";

    EXPECT_TRUE(afxLogFilePath("machines.log", &whyNot).has_value());
    EXPECT_EQ(whyNot, "untouched");
}
