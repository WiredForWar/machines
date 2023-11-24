/*
 * D B S Y S T E I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/dbsystei.hpp"
#include "machgui/dbplanet.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbISystem);

MachGuiDbISystem::MachGuiDbISystem()
{
    planets_.reserve(4);

    TEST_INVARIANT;
}

MachGuiDbISystem::~MachGuiDbISystem()
{
    TEST_INVARIANT;
}

void MachGuiDbISystem::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbISystem& t)
{

    o << "MachGuiDbISystem " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbISystem " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachGuiDbISystem& ob)
{
    ostr << ob.campaignPicture_;
    // ostr << ob.planets_;
    //  Write a copy without user scenarios
    ctl_vector<MachGuiDbPlanet*>* planets = new ctl_vector<MachGuiDbPlanet*>;
    for (MachGuiDbPlanet* planet : ob.planets_)
        if (!planet->isCustom())
            planets->push_back(planet);
    ostr << *planets;
    delete planets;
}

void perRead(PerIstream& istr, MachGuiDbISystem& ob)
{
    istr >> ob.campaignPicture_;
    istr >> ob.planets_;
}
/* End DBSYSTEI.CPP *************************************************/
