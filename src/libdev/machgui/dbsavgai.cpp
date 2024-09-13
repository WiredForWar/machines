/*
 * D B S A V G A I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/internal/dbsavgai.hpp"

#include "stdlib/string.hpp"
#include "machgui/dbplayer.hpp"
#include "machgui/dbscenar.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbISavedGame);

MachGuiDbISavedGame::MachGuiDbISavedGame()
    : pDbPlayer_(nullptr)
    , isCampaign_(false)
    , pDbScenario_(nullptr)
{

    TEST_INVARIANT;
}

MachGuiDbISavedGame::~MachGuiDbISavedGame()
{
    TEST_INVARIANT;
}

void MachGuiDbISavedGame::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbISavedGame& t)
{

    o << "MachGuiDbISavedGame " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbISavedGame " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void perWrite(PerOstream& ostr, const MachGuiDbISavedGame& ob)
{
    ostr << ob.userFileName_;
    ostr << ob.fileName_;
    ostr << ob.pDbPlayer_;
    ostr << ob.isCampaign_;
    ostr << ob.pDbScenario_;
}

void perRead(PerIstream& istr, MachGuiDbISavedGame& ob)
{
    istr >> ob.userFileName_;
    istr >> ob.fileName_;
    istr >> ob.pDbPlayer_;
    istr >> ob.isCampaign_;
    istr >> ob.pDbScenario_;
}

/* End DBSAVGAI.CPP *************************************************/
