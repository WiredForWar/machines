/*
 * S T R I N G . C P P
 * (c) Charybdis Limited, 1995. All Rights Reserved.
 */

//  Pre-instantiate basic_string for chars.

#define _INSTANTIATE_TEMPLATE_CLASSES

#include <ctype.h>
#include <iostream>
#include "stdlib/string.hpp"

PER_DEFINE_PERSISTENT_T1(basic_string, char);

template <class T> void perWrite(PerOstream& o, const basic_string<T>& str)
{
    PER_WRITE_RAW_OBJECT(o, str.length());
    o.write(str.data(), str.length());
}

template <class T> void perRead(PerIstream& i, basic_string<T>& str)
{
    size_t length;

    PER_READ_RAW_OBJECT(i, length);

    T* data = _NEW_ARRAY(T, length + 1);

    i.read(data, length);
    data[length] = '\0';

    str = data;

    _DELETE_ARRAY(data);
}

template void perRead<char>(PerIstream&, basic_string<char>&);
template void perWrite<char>(PerOstream&, const basic_string<char>&);

void writeAllocatedStringFromPointer(PerOstream& ostr, const string* pOb)
{
    PRE(pOb != nullptr);
    if (Persistence::instance().writePointerPre(
            ostr,
            (static_cast<const void*>(pOb)),
            "basic_stringchar",
            (static_cast<const void*>(pOb)),
            "basic_stringchar")
        == Persistence::WRITE_OBJECT)
    {
        ostr << *pOb;
    }
    Persistence ::instance().writePointerPost(ostr, (static_cast<const void*>(pOb)), "basic_stringchar", pOb);
}

void readAllocatedStringFromPointer(PerIstream& istr, string* pOb)
{
    PRE(pOb != nullptr);
    if (Persistence::instance().readPointerPre(istr, (reinterpret_cast<void**>(&pOb)), "basic_stringchar")
        == Persistence::READ_OBJECT)
    {
        istr >> *pOb;
    }
    else
    {
        pOb->clear();
    }
    Persistence::instance().readPointerPost(istr, (reinterpret_cast<void**>(&pOb)), "basic_stringchar");
}

void perWrite(PerOstream& ostr, const std::string& ob)
{
    PER_WRITE_RAW_OBJECT(ostr, ob.length());
    ostr.write(ob.data(), ob.length());
}

void perRead(PerIstream& istr, std::string& ob)
{
    std::size_t length;
    PER_READ_RAW_OBJECT(istr, length);
    ob.resize(length);
    istr.read(ob.data(), length);
}

PerOstream& operator<<(PerOstream& ostr, const std::string& ob)
{
    Persistence ::instance().writeObjectPre(ostr, (static_cast<const void*>(&ob)), "basic_string");
    perWrite(ostr, ob);
    Persistence ::instance().writeObjectPost((static_cast<const void*>(&ob)), "basic_string");
    return ostr;
}

PerIstream& operator>>(PerIstream& istr, std::string& ob)
{
    Persistence ::instance().readObjectPre(istr, (static_cast<const void*>(&ob)), "basic_string");
    perRead(istr, ob);
    Persistence ::instance().readObjectPost((static_cast<const void*>(&ob)), "basic_string");
    return istr;
}

/* End STRING.CPP ***************************************************/
