#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "stdlib/string.hpp"
#include "base/internal/persist.hpp"

TEST(PersistenceTests, ReadWriteString)
{
    std::ostringstream ofstr(std::ios::binary);
    PerOstream ostr(ofstr);

    std::string str = "my_text";
    ostr << str;

    constexpr PerIdentifier initialId = 100;
    using ClampedSizeType = uint32_t;

    ASSERT_EQ(ofstr.view().size(), sizeof(char) + sizeof(PerIdentifier) + sizeof(ClampedSizeType) + str.length());
    ASSERT_EQ(ofstr.view().size(), 16);
    const char *buffer = ofstr.view().data();

    // PerDataType
    ASSERT_EQ(buffer[0], static_cast<char>(PER_OBJECT));

    // Object PerIdentifier
    ASSERT_EQ(buffer[1], char(initialId));
    ASSERT_EQ(buffer[2], 0);
    ASSERT_EQ(buffer[3], 0);
    ASSERT_EQ(buffer[4], 0);

    // string size
    ASSERT_EQ(buffer[5], str.size());
    ASSERT_EQ(buffer[6], 0);
    ASSERT_EQ(buffer[7], 0);
    ASSERT_EQ(buffer[8], 0);

    for (std::size_t index = 0; index < str.size(); index++)
    {
        ASSERT_EQ(buffer[9 + index], str.at(index));
    }
}
