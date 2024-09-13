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

/* End STRING.CPP ***************************************************/
