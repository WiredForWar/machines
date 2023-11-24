/*
 * A C T W I N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "utility/linetok.hpp"
#include "sim/manager.hpp"
#include "machlog/actenabl.hpp"

PER_DEFINE_PERSISTENT(MachLogEnableActionAction);

MachLogEnableActionAction::MachLogEnableActionAction(SimCondition* pCondition, bool enabled)
    : SimAction(pCondition, enabled)
    , actionConditionKeyName_("")
{
    TEST_INVARIANT;
}

// virtual
MachLogEnableActionAction::~MachLogEnableActionAction()
{
    TEST_INVARIANT;
}

void MachLogEnableActionAction::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogEnableActionAction& t)
{

    o << "MachLogEnableActionAction " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogEnableActionAction " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachLogEnableActionAction::doAction()
{
    SimManager::instance().enableAction(actionConditionKeyName_);
}

// static
MachLogEnableActionAction*
MachLogEnableActionAction::newFromParser(SimCondition* pCondition, bool enabled, UtlLineTokeniser* pParser)
{
    MachLogEnableActionAction* pResult = nullptr;
    pResult = new MachLogEnableActionAction(pCondition, enabled);
    pResult->actionConditionKeyName_ = pParser->tokens()[3];
    return pResult;
}

// virtual
void MachLogEnableActionAction::doOutputOperator(std::ostream& o) const
{
    SimAction::doOutputOperator(o);
    o << "actionConditionKeyName_ " << actionConditionKeyName_ << std::endl;
}

void perWrite(PerOstream& ostr, const MachLogEnableActionAction& action)
{
    const SimAction& base1 = action;

    ostr << base1;
    ostr << action.actionConditionKeyName_;
}

void perRead(PerIstream& istr, MachLogEnableActionAction& action)
{
    SimAction& base1 = action;

    istr >> base1;
    istr >> action.actionConditionKeyName_;
}

MachLogEnableActionAction::MachLogEnableActionAction(PerConstructor con)
    : SimAction(con)
{
}

// static
MachLogEnableActionAction*
MachLogEnableActionAction::newDynamic(SimCondition* pCondition, bool enabled, const string& enableToken)
{
    MachLogEnableActionAction* pResult = nullptr;
    pResult = new MachLogEnableActionAction(pCondition, enabled);
    pResult->actionConditionKeyName_ = enableToken;
    return pResult;
}

/* End ACTREINF.CPP *************************************************/
