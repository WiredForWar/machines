/*
 * P E R S I S T . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "base/base.hpp"
#include "base/private/indent.hpp"
#include "base/lessptr.hpp"
#include "base/persist.hpp"
#include "base/internal/periwrit.hpp"
#include "base/internal/periread.hpp"

#include "ctl/map.hpp"

// static
Persistence& Persistence::instance()
{
    static Persistence instance_;
    return instance_;
}

Persistence::Persistence()
    : pImplementationWrite_(new PersistenceImplementationWrite)
    , pImplementationRead_(new PersistenceImplementationRead)
{
    TEST_INVARIANT;
}

Persistence::~Persistence()
{
    TEST_INVARIANT;

    delete pImplementationWrite_;
    delete pImplementationRead_;

    pImplementationRead_ = nullptr;
    pImplementationWrite_ = nullptr;
}

void Persistence::writeObjectPre(PerOstream& ostr, const void* pOb, const char* className)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writeObjectPre(ostr, pOb, className);
}

void Persistence::writeObjectPost(const void* pOb, const char* className)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writeObjectPost(pOb, className);
}

Persistence::PointerWriteResult Persistence::writePointerPre(
    PerOstream& ostr,
    const void* pOb,
    const char* className,
    const void* pMostDerivedOb,
    const char* mostDerivedClassName)
{
    PRE(pImplementationWrite_ != nullptr);

    return pImplementationWrite_->writePointerPre(ostr, pOb, className, pMostDerivedOb, mostDerivedClassName);
}

void Persistence::writePointerPost(PerOstream& ostr, const void* pOb, const char* className, const void* pMostDerivedOb)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writePointerPost(ostr, pOb, className, pMostDerivedOb);
}

void Persistence::writeRawPointer(PerOstream& ostr, const void* ptr)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writeRawPointer(ostr, ptr);
}

void Persistence::writeAsRaw(bool raw)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writeAsRaw(raw);
}

void Persistence::writeRawData(PerOstream& ostr, const char* ptr, size_t nBytes)
{
    PRE(pImplementationWrite_ != nullptr);

    pImplementationWrite_->writeRawData(ostr, ptr, nBytes);
}

void Persistence::readObjectPre(PerIstream& istr, const void* pOb, const char* className)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readObjectPre(istr, pOb, className);
}

void Persistence::readObjectPost(const void* pOb, const char* className)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readObjectPost(pOb, className);
}

Persistence::PointerReadResult Persistence::readPointerPre(PerIstream& istr, void** ptr, const char* className)
{
    PRE(pImplementationRead_ != nullptr);

    return pImplementationRead_->readPointerPre(istr, ptr, className);
}

void Persistence::readPointerPost(PerIstream& istr, void** ptr, const char* className)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readPointerPost(istr, ptr, className);
}

void Persistence::readRawPointer(PerIstream& istr, void** ptr)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readRawPointer(istr, ptr, "");
}

void Persistence::readPointerAbstract(PerIstream& istr, void** ptr, const char* className)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readPointerAbstract(istr, ptr, className);
}

void Persistence::readAsRaw(bool raw)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readAsRaw(raw);
}

void Persistence::readRawData(PerIstream& istr, char* ptr, size_t nBytes)
{
    PRE(pImplementationRead_ != nullptr);

    pImplementationRead_->readRawData(istr, ptr, nBytes);
}

void Persistence::registerDerivedClass(const char* className, PerWriteFnPtr writePtr, PerReadFnPtr readPtr)
{
    PRE(pImplementationRead_ != nullptr);
    PRE(pImplementationWrite_ != nullptr);

    pImplementationRead_->registerDerivedClass(className, readPtr);
    pImplementationWrite_->registerDerivedClass(className, writePtr);
}

std::ostream& Persistence::debugWriteStream()
{
    PRE(pImplementationWrite_ != nullptr);

    return pImplementationWrite_->debugStream();
}

std::ostream& Persistence::debugReadStream()
{
    PRE(pImplementationRead_ != nullptr);

    return pImplementationRead_->debugStream();
}

void Persistence::logAddresses(bool yesNo)
{
    PRE(pImplementationRead_ != nullptr);
    PRE(pImplementationWrite_ != nullptr);

    pImplementationRead_->logAddresses(yesNo);
    pImplementationWrite_->logAddresses(yesNo);
}

void Persistence::registerOpenIstream()
{
    pImplementationRead_->registerOpenIstream();
}

void Persistence::registerCloseIstream()
{
    pImplementationRead_->registerCloseIstream();
}

void Persistence::registerOpenOstream()
{
    pImplementationWrite_->registerOpenOstream();
}

void Persistence::registerCloseOstream()
{
    pImplementationWrite_->registerCloseOstream();
}

void Persistence::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// ostr.write( _STATIC_CAST( char*, &ob ), sizeof( ob ) );
#define PER_WRITE_BUILTIN_OBJECT(TYPE, STORED_TYPE)                                                                    \
    PerOstream& operator<<(PerOstream& ostr, const TYPE& ob)                                                           \
    {                                                                                                                  \
        STORED_TYPE obj = _STATIC_CAST(STORED_TYPE, ob);                                                               \
        Persistence::instance().writeObjectPre(ostr, &ob, #TYPE);                                                      \
        ostr.write(&obj, sizeof(obj));                                                                                 \
        Persistence::instance().writeObjectPost(&ob, #TYPE);                                                           \
        return ostr;                                                                                                   \
    }

#define PER_WRITE_BUILTIN_POINTER(TYPE)                                                                                \
    PerOstream& operator<<(PerOstream& ostr, const TYPE* pOb)                                                          \
    {                                                                                                                  \
        if (Persistence::instance().writePointerPre(ostr, pOb, #TYPE, pOb, #TYPE) == Persistence::WRITE_OBJECT)        \
        {                                                                                                              \
            ostr << *pOb;                                                                                              \
        }                                                                                                              \
        Persistence::instance().writePointerPost(ostr, pOb, #TYPE, pOb);                                               \
        return ostr;                                                                                                   \
    }

#define PER_READ_BUILTIN_OBJECT(TYPE, STORED_TYPE)                                                                     \
    PerIstream& operator>>(PerIstream& istr, TYPE& ob)                                                                 \
    {                                                                                                                  \
        STORED_TYPE obj;                                                                                               \
        Persistence::instance().readObjectPre(istr, &ob, #TYPE);                                                       \
        istr.read(&obj, sizeof(obj));                                                                                  \
        ob = _STATIC_CAST(TYPE, obj);                                                                                  \
        Persistence::instance().readObjectPost(&ob, #TYPE);                                                            \
        return istr;                                                                                                   \
    }

#define PER_READ_BUILTIN_POINTER(TYPE)                                                                                 \
    PerIstream& operator>>(PerIstream& istr, TYPE*& pOb)                                                               \
    {                                                                                                                  \
        if (Persistence::instance().readPointerPre(istr, _REINTERPRET_CAST(void**, &pOb), #TYPE)                       \
            == Persistence::READ_OBJECT)                                                                               \
        {                                                                                                              \
            istr >> *new TYPE;                                                                                       \
        }                                                                                                              \
        Persistence::instance().readPointerPost(istr, _REINTERPRET_CAST(void**, &pOb), #TYPE);                         \
        return istr;                                                                                                   \
    }

#define PER_READ_WRITE_RAW_BUILTIN_OBJECT(TYPE, STORED_TYPE)                                                           \
    void perWrite(PerOstream& ostr, const TYPE& ob)                                                                    \
    {                                                                                                                  \
        STORED_TYPE obj = _STATIC_CAST(STORED_TYPE, ob);                                                               \
        ostr.write(&obj, sizeof(obj));                                                                                 \
    }                                                                                                                  \
    void perRead(PerIstream& istr, TYPE& ob)                                                                           \
    {                                                                                                                  \
        STORED_TYPE obj;                                                                                               \
        istr.read(&obj, sizeof(obj));                                                                                  \
        ob = _STATIC_CAST(TYPE, obj);                                                                                  \
    }

#define PER_BUILTIN(TYPE, STORED_TYPE)                                                                                 \
    PER_WRITE_BUILTIN_OBJECT(TYPE, STORED_TYPE)                                                                        \
    PER_READ_BUILTIN_OBJECT(TYPE, STORED_TYPE)                                                                         \
    PER_WRITE_BUILTIN_POINTER(TYPE)                                                                                    \
    PER_READ_BUILTIN_POINTER(TYPE)                                                                                     \
    PER_READ_WRITE_RAW_BUILTIN_OBJECT(TYPE, STORED_TYPE)

// Charybdis bool was 4 bytes
PER_BUILTIN(bool, uint32_t);
PER_BUILTIN(char, int8_t);
PER_BUILTIN(signed char, int8_t);
PER_BUILTIN(unsigned char, uint8_t);
PER_BUILTIN(signed short, int16_t);
PER_BUILTIN(unsigned short, uint16_t);
PER_BUILTIN(signed int, int32_t);
PER_BUILTIN(unsigned int, uint32_t);
PER_BUILTIN(signed long, int32_t);
PER_BUILTIN(unsigned long, uint32_t);
PER_BUILTIN(unsigned long long, uint32_t);
PER_BUILTIN(float, float);
PER_BUILTIN(double, double);
PER_BUILTIN(long double, long double);

/* End PERSIST.CPP ******************************************************/
