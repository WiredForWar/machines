#include "stdlib/string.hpp"

static constexpr char stringPersistenceName[] = "basic_stringchar";

void writeAllocatedStringFromPointer(PerOstream& ostr, const std::string* pOb)
{
    PRE(pOb != nullptr);
    if (Persistence::instance().writePointerPre(ostr, pOb, stringPersistenceName, pOb, stringPersistenceName)
        == Persistence::WRITE_OBJECT)
    {
        ostr << *pOb;
    }
    Persistence ::instance().writePointerPost(ostr, pOb, stringPersistenceName, pOb);
}

void readAllocatedStringFromPointer(PerIstream& istr, std::string* pOb)
{
    PRE(pOb != nullptr);
    if (Persistence::instance().readPointerPre(istr, reinterpret_cast<void**>(&pOb), stringPersistenceName)
        == Persistence::READ_OBJECT)
    {
        istr >> *pOb;
    }
    else
    {
        pOb->clear();
    }
    Persistence::instance().readPointerPost(istr, reinterpret_cast<void**>(&pOb), stringPersistenceName);
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
