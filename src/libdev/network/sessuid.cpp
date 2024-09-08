#include "network/sessuid.hpp"

#include <cstring>

const NetAppSessionName& NetAppSessionUid::appSessionName() const
{
    return appSessionName_;
}

NetAppSessionUid::NetAppSessionUid(const NetAppSessionName& name)
    : appSessionName_(name)
{
}

//  Construct a NetAppSessionUid from raw data
NetAppSessionUid::NetAppSessionUid(uint8* buffer)
{
    size_t index = 0;

    size_t length;
    memcpy(&length, &buffer[index], sizeof(length));
    index += sizeof(length);

    char* str = _NEW_ARRAY(char, length + 1);
    memcpy(str, &buffer[index], length);
    str[length] = '\0';
    appSessionName_ = str;
    _DELETE_ARRAY(str);
    index += length;

    POST(index == rawBufferSize());
}

bool operator==(const NetAppSessionUid& lhs, const NetAppSessionUid& rhs)
{
    return lhs.appSessionName_ == rhs.appSessionName_;
}

size_t NetAppSessionUid::rawBufferSize() const
{
    const size_t size = sizeof(std::size_t) + appSessionName_.length();

    return size;
}

//  Convert a NetAppSessionUid into raw data
void NetAppSessionUid::rawBuffer(uint8* buffer) const
{
    size_t index = 0;

    const std::size_t length = appSessionName_.length();
    memcpy(&buffer[index], &length, sizeof(length));
    index += sizeof(length);

    memcpy(&buffer[index], appSessionName_.c_str(), length);
    index += length;

    POST(index == rawBufferSize());
}
