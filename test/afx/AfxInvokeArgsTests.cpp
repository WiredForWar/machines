#include "afx/invkargs.hpp"

#include <gtest/gtest.h>

namespace
{

AfxInvokeArgs argsFrom(std::initializer_list<const char*> tokens)
{
    AfxInvokeArgs args;
    for (const char* token : tokens)
        args.emplace_back(token);

    return args;
}

} // namespace

TEST(AfxInvokeArgsTests, ContainsMatchesAWholeToken)
{
    const AfxInvokeArgs args = argsFrom({ "-lobby", "--log-to-console" });

    EXPECT_TRUE(args.contains("-lobby"));
    EXPECT_TRUE(args.contains("--log-to-console"));
    EXPECT_FALSE(args.contains("--log"));
    EXPECT_FALSE(args.contains("lobby"));
}

TEST(AfxInvokeArgsTests, ContainsIgnoresAFlagGivenAValue)
{
    const AfxInvokeArgs args = argsFrom({ "--test-server=19876" });

    EXPECT_FALSE(args.contains("--test-server"));
}

TEST(AfxInvokeArgsTests, ValueReadsTheTextAfterTheEquals)
{
    const AfxInvokeArgs args = argsFrom({ "--test-server=19876" });

    ASSERT_TRUE(args.value("--test-server").has_value());
    EXPECT_EQ(args.value("--test-server").value(), "19876");
}

TEST(AfxInvokeArgsTests, ValueIsEmptyForAFlagGivenOnItsOwn)
{
    const AfxInvokeArgs args = argsFrom({ "--test-server" });

    EXPECT_FALSE(args.value("--test-server").has_value());
}

TEST(AfxInvokeArgsTests, ValueAcceptsAnEmptyText)
{
    const AfxInvokeArgs args = argsFrom({ "--test-server=" });

    ASSERT_TRUE(args.value("--test-server").has_value());
    EXPECT_TRUE(args.value("--test-server").value().empty());
}

TEST(AfxInvokeArgsTests, ValueDoesNotMatchALongerFlag)
{
    const AfxInvokeArgs args = argsFrom({ "--test-server-port=19876" });

    EXPECT_FALSE(args.value("--test-server").has_value());
}
