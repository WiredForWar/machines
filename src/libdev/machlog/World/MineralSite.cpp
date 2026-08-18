/*
 * M I N E S I T E. C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#include "machlog/World/MineralSite.hpp"

#include "machlog/Internal/MineralSiteImpl.hpp"

#include "ctl/Algorithm.hpp"
#include "ctl/List.hpp"
#include "mathex/Point2d.hpp"

#include "machlog/Actors/ActorMaker.hpp"
#include "machlog/Messaging/MessageBroker.hpp"
#include "machlog/Actors/Mine.hpp"
#include "machlog/Messaging/Network.hpp"
#include "machlog/Actors/OreHolograph.hpp"
#include "machlog/World/Planet.hpp"
#include "machlog/Race.hpp"
#include "machlog/Races.hpp"
#include "machlog/Messaging/VoiceMailManager.hpp"
#include "machlog/Messaging/VoiceMailData.hpp"

/* //////////////////////////////////////////////////////////////// */
PER_DEFINE_PERSISTENT(MachLogMineralSite);

#define CB_MachLogMineralSite_DEPIMPL()                                                                                \
    PRE(pImpl_)                                                                                                        \
    CB_DEPIMPL(MachLogMineralSite::State, state_)                                                                      \
    CB_DEPIMPL(MexPoint3d, position_)                                                                                  \
    CB_DEPIMPL(MachPhys::BuildingMaterialUnits, amountOfOre_)                                                          \
    CB_DEPIMPL(MachPhys::MineralGrade, grade_)                                                                         \
    CB_DEPIMPL(bool, registeredAsExhausted_)                                                                           \
    CB_DEPIMPL(MachLogOreHolograph*, pOreHolo_)                                                                        \
    CB_DEPIMPL(int, id_)                                                                                               \
    CB_DEPIMPL(MachPhys::Race, discoveredBy_);

MachLogMineralSite::MachLogMineralSite(
    const MachPhys::MineralGrade& grade,
    const MachPhys::BuildingMaterialUnits& amount,
    const MexPoint3d& p)
    : pImpl_(new MachLogMineralSiteImpl(grade, amount, p))
{
    CB_MachLogMineralSite_DEPIMPL();

    PRE(amountOfOre_ > 0);
    PRE(grade_ > 0 && grade < 5);
    MachLogPlanet::instance().newSite(this);
}

MachLogMineralSite::~MachLogMineralSite()
{
    delete pImpl_;
}

void MachLogMineralSite::beDiscoveredBy(MachPhys::Race race)
{
    CB_MachLogMineralSite_DEPIMPL();

    ASSERT(state_ == UNDISCOVERED, logic_error());
    //  MachLogRace * pRace = &MachLogRaces::instance().race( race );
    //  ctl_append( &MachLogRaces::instance().holographs( race ),

    state_ = DISCOVERED;
    discoveredBy_ = race;

    pOreHolo_ = MachLogActorMaker::newLogOreHolograph(race, 0, 0, position());

    // if the race discovering this site is remote then do NOT create an ore holograph as one will be created for us.
    if (MachLogNetwork::instance().isNetworkGame()
        && MachLogNetwork::instance().remoteStatus(race) == MachLogNetwork::REMOTE_PROCESS)
    {
    }
    else
    {
        MachLogMessageBroker& broker = MachLogMessageBroker::instance();
        if (broker.isPublishing())
            broker.sendSetSiteDiscoveredByMessage(position_, race);
    }
    //  new MachLogOreHolograph( pRace, 0, 0, position() );

    // call race method to notify all fristd::endly mines of this site's newly-discovered state
    // as they may now wish to make use of it if currently without a site.
    MachLogRaces::instance().newMineralSiteDiscovered(race);
};

MachPhys::BuildingMaterialUnits MachLogMineralSite::amountOfOre() const
{
    CB_MachLogMineralSite_DEPIMPL();

    return amountOfOre_;
}

MachPhys::BuildingMaterialUnits MachLogMineralSite::extractOre(MachPhys::BuildingMaterialUnits oreExtracted)
{
    // oreExtracted should be understood to mean "ore I am TRYING to extract". May not be able to
    // extract this much if the mineral site doesn't have that much there. The amount that actually has been
    // extracted is the return value of the method.

    CB_MachLogMineralSite_DEPIMPL();

    PRE(oreExtracted >= 0);

    if (oreExtracted > amountOfOre_)
        oreExtracted = amountOfOre_;

    MachPhys::BuildingMaterialUnits newOreAmount = amountOfOre_ - oreExtracted;

    setOre(newOreAmount);

    MachLogMessageBroker& broker = MachLogMessageBroker::instance();
    if (broker.isPublishing())
        broker.sendSetSiteOreMessage(id(), newOreAmount);

    return oreExtracted;
}

void MachLogMineralSite::setOre(MachPhys::BuildingMaterialUnits amountOfOre)
{
    CB_MachLogMineralSite_DEPIMPL();

    amountOfOre_ = amountOfOre;

    if (amountOfOre_ == 0)
    {
        if (! registeredAsExhausted_)
        {
            registeredAsExhausted_ = true;

            // delete the ore holo from existence
            if (pOreHolo_)
            {
                pOreHolo_->removeMe();
                pOreHolo_ = nullptr;
            }

            // need to deal with any mines that may have been using this mineral site

            bool pcPlayerHadMineUsingThis = false;

            for (MachPhys::Race race : MachPhys::AllRaces)
            {
                const MachLogRaces::Mines& mines = MachLogRaces::instance().mines(race);
                if (mines.size() > 0)
                {

                    for (MachLogRaces::Mines::const_iterator i = mines.begin(); i != mines.end(); ++i)
                    {
                        MachLogMine& mine = (**i);
                        if (mine.hasMineralSite() && &(const_cast<const MachLogMine&>(mine).mineralSite()) == this)
                        {
                            // need to give voicemail if appropriate.
                            if (! pcPlayerHadMineUsingThis
                                && mine.race() == MachLogRaces::instance().playerRace())
                            {
                                pcPlayerHadMineUsingThis = true;
                                MachLogVoiceMailManager::instance().postNewMail(
                                    VID_POD_MINERALS_EXHAUSTED,
                                    position_,
                                    MachLogRaces::instance().playerRace());
                            }
                            mine.mineralSiteIsExhausted();
                        }
                    }
                }
            }
        }
    }
}

MachPhys::MineralGrade MachLogMineralSite::grade() const
{
    CB_MachLogMineralSite_DEPIMPL();

    return grade_;
}

bool MachLogMineralSite::hasBeenDiscoveredBy(MachPhys::Race race) const
{
    CB_MachLogMineralSite_DEPIMPL();

    return state_ == DISCOVERED && discoveredBy_ == race;
}

bool MachLogMineralSite::hasBeenDiscovered() const
{
    CB_MachLogMineralSite_DEPIMPL();

    return state_ == DISCOVERED;
}

const MexPoint3d& MachLogMineralSite::position() const
{
    CB_MachLogMineralSite_DEPIMPL();

    return position_;
}

std::ostream& operator<<(std::ostream& o, const MachLogMineralSite& t)
{
    o << "Mineral site:" << std::endl;
    o << " Position " << t.position() << std::endl;
    o << " Amount of Ore " << t.amountOfOre() << std::endl;
    o << " Mineral Grade " << t.grade() << std::endl;
    o << " discovered " << (t.hasBeenDiscovered() ? "True" : "False") << std::endl;
    if (t.hasBeenDiscovered())
        o << "  by race " << (int)t.discoveredByRace() << std::endl;
    o << " " << static_cast<const void*>(&t) << " END ";
    return o;
}

MachPhys::Race MachLogMineralSite::discoveredByRace() const
{
    CB_MachLogMineralSite_DEPIMPL();

    PRE(hasBeenDiscovered());
    return discoveredBy_;
}

void MachLogMineralSite::id(int idVal)
{
    CB_MachLogMineralSite_DEPIMPL();

    id_ = idVal;
}

int MachLogMineralSite::id() const
{
    CB_MachLogMineralSite_DEPIMPL();

    return id_;
}

/* //////////////////////////////////////////////////////////////// */
void perWrite(PerOstream& ostr, const MachLogMineralSite& site)
{
    ostr << site.pImpl_;
}

void perRead(PerIstream& istr, MachLogMineralSite& site)
{
    istr >> site.pImpl_;
}

MachLogMineralSite::MachLogMineralSite(PerConstructor)
{
}

/* End MINESITE.CPP *************************************************/
