#include <gtest/gtest.h>

#include "network/AddressUtils.hpp"

TEST(NetUtilsTests, NoArgsIsUnsetTest)
{
    std::string addressStr = "127.0.0.1:1234";
    auto port = getPort(addressStr);
    auto ip = getIp(addressStr);

    ASSERT_TRUE(port.has_value());
    ASSERT_EQ(port.value(), 1234u);
    ASSERT_EQ(ip, "127.0.0.1");
}
