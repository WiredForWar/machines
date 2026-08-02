#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "base/internal/Persistence.hpp"
#include "base/Persistence.hpp"
#include "ctl/PtrVector.hpp"
#include "ctl/Vector.hpp"

constexpr PerIdentifier initialId = 100;
using ClampedSizeType = uint32_t;

//  A minimal persistent class holding a pointer to another instance of itself.
//  Writing an instance whose pointer is null makes PER_POINTER_WRITE pass a null
//  most derived class name down into the persistence implementation.
class PerNullPointerHost;
void perWrite(PerOstream& ostr, const PerNullPointerHost& ob);
void perRead(PerIstream& istr, PerNullPointerHost& ob);

class PerNullPointerHost
{
public:
    PerNullPointerHost() = default;

    PER_MEMBER_PERSISTENT_DEFAULT_INLINE(PerNullPointerHost);

    PerNullPointerHost* pOther_ = nullptr;
};

void perWrite(PerOstream& ostr, const PerNullPointerHost& ob)
{
    ostr << ob.pOther_;
}

void perRead(PerIstream& istr, PerNullPointerHost& ob)
{
    istr >> ob.pOther_;
}

TEST(PersistenceTests, WriteNullObjectPointer)
{
    std::ostringstream ofstr(std::ios::binary);
    PerOstream ostr(ofstr);

    PerNullPointerHost host;
    ASSERT_EQ(host.pOther_, nullptr);

    ostr << host;

    //  The object header plus the PER_POINTER marker and the pointer identifier
    ASSERT_EQ(ofstr.view().size(), 2 * (sizeof(char) + sizeof(PerIdentifier)));
    const char* buffer = ofstr.view().data();

    EXPECT_EQ(buffer[0], static_cast<char>(PER_OBJECT));

    //  A null pointer is written as a plain PER_POINTER, without the object
    EXPECT_EQ(buffer[sizeof(char) + sizeof(PerIdentifier)], static_cast<char>(PER_POINTER));
}

TEST(PersistenceTests, WriteString)
{
    std::ostringstream ofstr(std::ios::binary);
    PerOstream ostr(ofstr);

    std::string str = "my_text";
    ostr << str;

    EXPECT_EQ(ofstr.view().size(), sizeof(char) + sizeof(PerIdentifier) + sizeof(ClampedSizeType) + str.length());
    EXPECT_EQ(ofstr.view().size(), 16);
    const char *buffer = ofstr.view().data();

    // PerDataType
    EXPECT_EQ(buffer[0], static_cast<char>(PER_OBJECT));

    // Object PerIdentifier
    EXPECT_EQ(buffer[1], static_cast<char>(initialId));
    EXPECT_EQ(buffer[2], 0);
    EXPECT_EQ(buffer[3], 0);
    EXPECT_EQ(buffer[4], 0);

    // string size
    EXPECT_EQ(buffer[5], str.size());
    EXPECT_EQ(buffer[6], 0);
    EXPECT_EQ(buffer[7], 0);
    EXPECT_EQ(buffer[8], 0);

    for (std::size_t index = 0; index < str.size(); index++)
    {
        EXPECT_EQ(buffer[9 + index], str.at(index));
    }
}

TEST(PersistenceTests, ReadString)
{
    const std::string referenceData = "my_text";
    ASSERT_EQ(static_cast<std::string::size_type>(static_cast<char>(referenceData.size())), referenceData.size());

    static const char buffer[] = {
        // PerDataType
        PER_OBJECT,

        // Object PerIdentifier
        static_cast<char>(initialId), 0, 0, 0,

        // string size
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // the string content
        referenceData[0],
        referenceData[1],
        referenceData[2],
        referenceData[3],
        referenceData[4],
        referenceData[5],
    };
    std::stringstream buf;
    buf.write(buffer, sizeof(buffer));
    ASSERT_EQ(std::string_view(buffer, sizeof(buffer)), buf.rdbuf()->view());

    PerIstream istr(buf);

    std::string str;
    istr >> str;

    EXPECT_EQ(str.size(), referenceData.size());
}

TEST(PersistenceTests, WriteEmptyIntVector)
{
    static const char buffer[] = {
        // PerDataType
        PER_OBJECT,

        // Object PerIdentifier
        static_cast<char>(initialId), 0, 0, 0,

        // vector size
        0, 0, 0, 0,

        // vector reserved
        0, 0, 0, 0,

        // vector capacity
        0, 0, 0, 0,
    };

    std::stringstream buf;
    buf.write(buffer, sizeof(buffer));
    ASSERT_EQ(std::string_view(buffer, sizeof(buffer)), buf.rdbuf()->view());

    {
        std::ostringstream ofstr(std::ios::binary);
        PerOstream ostr(ofstr);

        ctl_vector<int> data;
        ostr << data;

        EXPECT_EQ(ofstr.view().size(), buf.rdbuf()->view().size());
        EXPECT_EQ(std::string_view(buffer, sizeof(buffer)), ofstr.view());
    }

    {
        PerIstream istr(buf);

        ctl_vector<int> data;
        istr >> data;

        EXPECT_EQ(data.size(), 0);
    }
}

TEST(PersistenceTests, ReadWriteIntCTLVector)
{
    const std::vector<int> referenceData { 0x00112233, 0x44556677 };
    static const char buffer[] = {
        // PerDataType
        PER_OBJECT,

        // Object PerIdentifier
        static_cast<char>(initialId), 0, 0, 0,

        // vector size
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // vector reserved
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // vector capacity
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 1), 0, 0, 0,
        0x33, 0x22, 0x11, 0x00,

        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 2), 0, 0, 0,
        0x77, 0x66, 0x55, 0x44,
    };

    std::stringstream buf;
    buf.write(buffer, sizeof(buffer));
    ASSERT_EQ(std::string_view(buffer, sizeof(buffer)), buf.rdbuf()->view());

    {
        std::ostringstream ofstr(std::ios::binary);
        PerOstream ostr(ofstr);

        ctl_vector<int> data {
            referenceData[0],
            referenceData[1],
        };
        ostr << data;

        EXPECT_EQ(ofstr.view().size(), buf.rdbuf()->view().size());
        EXPECT_EQ(std::string_view(buffer, sizeof(buffer)), ofstr.view());
    }

    {
        buf.seekg(0);
        PerIstream istr(buf);

        ctl_vector<int> data;
        istr >> data;

        EXPECT_EQ(data.size(), referenceData.size());
        ASSERT_EQ(data.size(), 2);
        EXPECT_EQ(data[0], referenceData[0]);
        EXPECT_EQ(data[1], referenceData[1]);
    }
}

TEST(PersistenceTests, ReadWriteIntPointersCTLVector)
{
    std::vector<int> referenceData { 0x00112233, 0x44556677 };
    static const char buffer[] = {
        // PerDataType
        PER_OBJECT,

        // Object PerIdentifier
        static_cast<char>(initialId), 0, 0, 0,

        // vector size
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // vector reserved
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // vector capacity
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // 'Pointer pre'
        PER_OBJECT_POINTER,
        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 1), 0, 0, 0,
        0x33, 0x22, 0x11, 0x00,
        // Object 'Pointer post' PerIdentifier
        static_cast<char>(initialId + 1), 0, 0, 0,

        // 'Pointer pre'
        PER_OBJECT_POINTER,
        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 2), 0, 0, 0,
        0x77, 0x66, 0x55, 0x44,
        // Object 'Pointer post' PerIdentifier
        static_cast<char>(initialId + 2), 0, 0, 0,
    };

    std::stringstream buf;
    buf.write(buffer, sizeof(buffer));
    ASSERT_EQ(std::string_view(buffer, sizeof(buffer)), buf.rdbuf()->view());

    {
        std::ostringstream ofstr(std::ios::binary);
        PerOstream ostr(ofstr);

        ctl_vector<int*> data {
            &referenceData[0],
            &referenceData[1],
        };
        ostr << data;

        EXPECT_EQ(ofstr.view().size(), buf.rdbuf()->view().size());
        EXPECT_EQ(std::string_view(buffer, sizeof(buffer)), ofstr.view());
    }

    {
        buf.seekg(0);
        PerIstream istr(buf);

        ctl_vector<int*> data;
        istr >> data;

        EXPECT_EQ(data.size(), referenceData.size());
        ASSERT_EQ(data.size(), 2);
        EXPECT_EQ(*data[0], referenceData[0]);
        EXPECT_EQ(*data[1], referenceData[1]);

        for (auto* p : data)
        {
            delete p;
        }
    }
}

TEST(PersistenceTests, ReadWriteIntCTLPVector)
{
    std::vector<int> referenceData { 0x00112233, 0x44556677 };
    static const char buffer[] = {
        // PerDataType
        PER_OBJECT,

        // Object PerIdentifier
        static_cast<char>(initialId), 0, 0, 0,

        // vector size
        static_cast<char>(referenceData.size()), 0, 0, 0,

        // 'Pointer pre'
        PER_OBJECT_POINTER,
        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 1), 0, 0, 0,
        0x33, 0x22, 0x11, 0x00,
        // Object 'Pointer post' PerIdentifier
        static_cast<char>(initialId + 1), 0, 0, 0,

        // 'Pointer pre'
        PER_OBJECT_POINTER,
        // PerDataType
        PER_OBJECT,
        // Object PerIdentifier
        static_cast<char>(initialId + 2), 0, 0, 0,
        0x77, 0x66, 0x55, 0x44,
        // Object 'Pointer post' PerIdentifier
        static_cast<char>(initialId + 2), 0, 0, 0,
    };

    std::stringstream buf;
    buf.write(buffer, sizeof(buffer));
    ASSERT_EQ(std::string_view(buffer, sizeof(buffer)), buf.rdbuf()->view());

    {
        std::ostringstream ofstr(std::ios::binary);
        PerOstream ostr(ofstr);

        ctl_pvector<int> data {
            &referenceData[0],
            &referenceData[1],
        };
        ostr << data;

        EXPECT_EQ(ofstr.view().size(), buf.rdbuf()->view().size());
        EXPECT_EQ(std::string_view(buffer, sizeof(buffer)), ofstr.view());
    }

    {
        buf.seekg(0);
        PerIstream istr(buf);

        ctl_pvector<int> data;
        istr >> data;

        EXPECT_EQ(data.size(), referenceData.size());
        ASSERT_EQ(data.size(), 2);
        EXPECT_EQ(*data[0], referenceData[0]);
        EXPECT_EQ(*data[1], referenceData[1]);

        for (auto* p : data)
        {
            delete p;
        }
    }
}
