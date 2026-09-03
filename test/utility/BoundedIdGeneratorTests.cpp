#include <gtest/gtest.h>

#include "utility/BoundedIdGenerator.hpp"

#include <optional>
#include <set>

TEST(BoundedIdGeneratorTests, HandsOutEveryIdInThePoolExactlyOnce)
{
    constexpr UtlId poolSize = 8;
    UtlBoundedIdGenerator generator(poolSize);

    std::set<UtlId> seen;
    for (UtlId i = 0; i != poolSize; ++i)
    {
        const std::optional<UtlId> id = generator.nextId();
        ASSERT_TRUE(id.has_value());
        EXPECT_LT(id.value(), poolSize);
        EXPECT_TRUE(seen.insert(id.value()).second);
    }

    EXPECT_EQ(poolSize, seen.size());
    EXPECT_EQ(0u, generator.nUnusedIds());
}

TEST(BoundedIdGeneratorTests, YieldsNoIdOnceThePoolIsExhausted)
{
    constexpr UtlId poolSize = 4;
    UtlBoundedIdGenerator generator(poolSize);

    for (UtlId i = 0; i != poolSize; ++i)
        ASSERT_TRUE(generator.nextId().has_value());

    EXPECT_FALSE(generator.nextId().has_value());
}

TEST(BoundedIdGeneratorTests, ReusesAnIdThatHasBeenFreed)
{
    constexpr UtlId poolSize = 4;
    UtlBoundedIdGenerator generator(poolSize);

    std::optional<UtlId> first = generator.nextId();
    ASSERT_TRUE(first.has_value());

    for (UtlId i = 1; i != poolSize; ++i)
        ASSERT_TRUE(generator.nextId().has_value());

    generator.freeId(first.value());
    EXPECT_EQ(first, generator.nextId());
}

// A free count that has drifted above the number of unallocated flags used to
// leave nextId() scanning a full array for ever. It has to run out instead.
TEST(BoundedIdGeneratorTests, YieldsNoIdWhenTheFreeCountOverstatesThePool)
{
    constexpr UtlId poolSize = 4;
    UtlBoundedIdGenerator generator(poolSize);

    const std::optional<UtlId> id = generator.nextId();
    ASSERT_TRUE(id.has_value());

    // Releasing the same id twice is what drives the count out of step
    generator.freeId(id.value());
    generator.freeId(id.value());

    for (UtlId i = 0; i != poolSize; ++i)
        ASSERT_TRUE(generator.nextId().has_value());

    EXPECT_NE(0u, generator.nUnusedIds());
    EXPECT_FALSE(generator.nextId().has_value());
}

TEST(BoundedIdGeneratorTests, KeepsARangedIdInsideItsRange)
{
    constexpr UtlId poolSize = 32;
    constexpr UtlId minId = 8;
    constexpr UtlId maxId = 12;
    UtlBoundedIdGenerator generator(poolSize);

    std::set<UtlId> seen;
    for (UtlId i = minId; i != maxId; ++i)
    {
        const std::optional<UtlId> id = generator.nextId(minId, maxId);
        ASSERT_TRUE(id.has_value());
        EXPECT_GE(id.value(), minId);
        EXPECT_LT(id.value(), maxId);
        EXPECT_TRUE(seen.insert(id.value()).second);
    }

    EXPECT_EQ(maxId - minId, seen.size());
}

TEST(BoundedIdGeneratorTests, YieldsNoIdOnceARangeIsExhausted)
{
    constexpr UtlId poolSize = 32;
    constexpr UtlId minId = 8;
    constexpr UtlId maxId = 12;
    UtlBoundedIdGenerator generator(poolSize);

    for (UtlId i = minId; i != maxId; ++i)
        ASSERT_TRUE(generator.nextId(minId, maxId).has_value());

    // The rest of the pool is free, but none of it is inside the range
    EXPECT_FALSE(generator.nextId(minId, maxId).has_value());
    EXPECT_NE(0u, generator.nUnusedIds());
}
