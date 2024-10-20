/*
 * A C T W I N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "mathex/point2d.hpp"
#include "utility/linetok.hpp"
#include "sim/manager.hpp"
#include "machlog/actwin.hpp"
#include "machlog/scenario.hpp"
#include "machlog/races.hpp"

PER_DEFINE_PERSISTENT(MachLogWinAction);

MachLogWinAction::MachLogWinAction(SimCondition* pCondition, bool enabled)
    : SimAction(pCondition, enabled)
{
    TEST_INVARIANT;
}

// virtual
MachLogWinAction::~MachLogWinAction()
{
    TEST_INVARIANT;
}

void MachLogWinAction::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogWinAction& t)
{

    o << "MachLogWinAction " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogWinAction " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

// virtual
void MachLogWinAction::doAction()
{
    MachLogRaces::instance().hasWon(race_, true);
}

// static
std::unique_ptr<MachLogWinAction> MachLogWinAction::newFromParser(SimCondition* pCondition, bool enabled, UtlLineTokeniser* pParser)
{
    std::unique_ptr<MachLogWinAction> pResult(new MachLogWinAction(pCondition, enabled));
    for (std::size_t i = 0; i < pParser->tokens().size(); ++i)
    {
        const std::string& token = pParser->tokens()[i];
        if (token == "RACE")
            pResult->race_ = MachLogScenario::machPhysRace(pParser->tokens()[i + 1]);
    }
    return pResult;
}

// virtual
void MachLogWinAction::doOutputOperator(std::ostream& o) const
{
    SimAction::doOutputOperator(o);
    o << "Race " << race_ << std::endl;
}

void perWrite(PerOstream& ostr, const MachLogWinAction& action)
{
    const SimAction& base1 = action;

    ostr << base1;
    ostr << action.race_;
}

void perRead(PerIstream& istr, MachLogWinAction& action)
{
    SimAction& base1 = action;

    istr >> base1;
    istr >> action.race_;
}

MachLogWinAction::MachLogWinAction(PerConstructor con)
    : SimAction(con)
{
}

// static
std::unique_ptr<MachLogWinAction>
MachLogWinAction::newDynamic(SimCondition* pCondition, bool enabled, MachPhys::Race race)
{
    std::unique_ptr<MachLogWinAction> pResult(new MachLogWinAction(pCondition, enabled));
    pResult->race_ = race;
    return pResult;
}

/* End ACTREINF.CPP *************************************************/
