#include "PersistenceFlagsInBool.hpp"

PerOstream& operator<<(PerOstream& ostr, const PersistenceFlagsInBool& ob)
{
    uint32_t obj = ob.value;
    Persistence ::instance().writeObjectPre(ostr, ob.perObject, "bool");
    ostr.write(&obj, sizeof(obj));
    Persistence ::instance().writeObjectPost(&ob, "bool");
    return ostr;
}

PerIstream& operator>>(PerIstream& istr, PersistenceFlagsInBool& ob)
{
    uint32_t& obj = ob.value;
    Persistence ::instance().readObjectPre(istr, ob.perObject, "bool");
    istr.read(&obj, sizeof(obj));
    Persistence ::instance().readObjectPost(&ob, "bool");
    return istr;
}
