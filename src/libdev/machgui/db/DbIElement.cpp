/*
 * D B E L E M E I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/db/internal/DbIElement.hpp"
#include "machgui/db/DbElement.hpp"

// The following include is needed for std::unique_ptr<MachGuiDbTextData> pTextData_ destructor
#include "machgui/db/DbTextData.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbIElement);

MachGuiDbIElement::MachGuiDbIElement()
{
    // Use sensible collection sizes
    antecedents_.reserve(4);

    TEST_INVARIANT;
}

MachGuiDbIElement::~MachGuiDbIElement()
{
    TEST_INVARIANT;
}

void MachGuiDbIElement::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbIElement& t)
{

    o << "MachGuiDbIElement " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbIElement " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachGuiDbIElement& ob)
{
    ostr << ob.antecedents_;
    ostr << ob.menuStringId_;
    ostr << ob.name_;
    ostr << ob.textDataFileName_;
}

void perRead(PerIstream& istr, MachGuiDbIElement& ob)
{
    istr >> ob.antecedents_;
    istr >> ob.menuStringId_;
    istr >> ob.name_;
    istr >> ob.textDataFileName_;

    ob.isComplete_ = false;
    ob.isCustom_ = false;
    ob.pTextData_ = nullptr;
}

/* End DBELEMEI.CPP *************************************************/
