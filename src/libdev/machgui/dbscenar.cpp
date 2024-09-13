/*
 * D B S C E N A R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dbscenar.hpp"
#include "machgui/database.hpp"
#include "machgui/dbplanet.hpp"
#include "machgui/dbsystem.hpp"
#include "machgui/internal/dbscenai.hpp"

PER_DEFINE_PERSISTENT(MachGuiDbScenario);

MachGuiDbScenario::MachGuiDbScenario(
    const std::string& scenarioName,
    const std::string& planetName,
    uint menuStringId,
    uint maxPlayers)
    : MachGuiDbElement(menuStringId)
{
    initialise(scenarioName, planetName);
    pData_->maxPlayers_ = maxPlayers;

    TEST_INVARIANT;
}

void MachGuiDbScenario::initialise(const std::string& scenarioName, const std::string& planetName)
{
    pData_ = new MachGuiDbIScenario;
    name(scenarioName);
    planetFile(planetName);

    TEST_INVARIANT;
}

MachGuiDbScenario::~MachGuiDbScenario()
{
    TEST_INVARIANT;

    delete pData_;
}

void MachGuiDbScenario::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiDbScenario& t)
{

    o << "MachGuiDbScenario " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiDbScenario " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiDbScenario::campaignPicture(const std::string& filename)
{
    pData_->campaignPicture_ = filename;
}

const std::string& MachGuiDbScenario::campaignPicture() const
{
    return pData_->campaignPicture_;
}

void MachGuiDbScenario::briefingPicture(const std::string& filename)
{
    pData_->briefingPicture_ = filename;
}

const std::string& MachGuiDbScenario::briefingPicture() const
{
    return pData_->briefingPicture_;
}

void MachGuiDbScenario::debriefingPicture(const std::string& filename)
{
    pData_->debriefingPicture_ = filename;
}

const std::string& MachGuiDbScenario::debriefingPicture() const
{
    return pData_->debriefingPicture_;
}

void MachGuiDbScenario::entryFlic(const std::string& filename)
{
    pData_->entryFlic_ = filename;
}

const std::string& MachGuiDbScenario::entryFlic() const
{
    return pData_->entryFlic_;
}

void MachGuiDbScenario::winFlic(const std::string& filename)
{
    pData_->winFlic_ = filename;
}

const std::string& MachGuiDbScenario::winFlic() const
{
    return pData_->winFlic_;
}

void MachGuiDbScenario::loseFlic(const std::string& filename)
{
    pData_->loseFlic_ = filename;
}

const std::string& MachGuiDbScenario::loseFlic() const
{
    return pData_->loseFlic_;
}

void MachGuiDbScenario::planet(MachGuiDbPlanet* pPlanet)
{
    pData_->pPlanet_ = pPlanet;
}

MachGuiDbPlanet& MachGuiDbScenario::planet() const
{
    PRE(pData_->pPlanet_ != nullptr);
    return *pData_->pPlanet_;
}

uint MachGuiDbScenario::maxPlayers() const
{
    return pData_->maxPlayers_;
}

void perWrite(PerOstream& ostr, const MachGuiDbScenario& ob)
{
    const MachGuiDbElement& base = ob;

    ostr << base;

    ostr << ob.pData_;
}

void perRead(PerIstream& istr, MachGuiDbScenario& ob)
{
    MachGuiDbElement& base = ob;

    istr >> base;

    istr >> ob.pData_;
}

MachGuiDbScenario::MachGuiDbScenario(PerConstructor con)
    : MachGuiDbElement(con)
    , pData_(nullptr)
{
}

void MachGuiDbScenario::planetFile(const std::string& filename)
{
    pData_->planetFile_ = filename;
}

const std::string& MachGuiDbScenario::planetFile() const
{
    return pData_->planetFile_;
}

uint MachGuiDbScenario::nRacesToHaveSurvivingMachinesSaved() const
{
    return pData_->saveMachineRaces_.size();
}

MachPhys::Race MachGuiDbScenario::raceToHaveSurvivingMachinesSaved(uint index)
{
    PRE(index < nRacesToHaveSurvivingMachinesSaved());
    return pData_->saveMachineRaces_[index];
}

void MachGuiDbScenario::raceToHaveSurvivingMachinesSaved(MachPhys::Race race)
{
    pData_->saveMachineRaces_.push_back(race);
}

uint MachGuiDbScenario::musicTrack() const
{
    return pData_->musicTrack_;
}

void MachGuiDbScenario::musicTrack(uint track)
{
    pData_->musicTrack_ = track;
}

bool MachGuiDbScenario::isTrainingScenario() const
{
    bool retVal = false;

    if (&planet().system() == &MachGuiDatabase::instance().campaignSystem(0))
    {
        retVal = true;
    }

    return retVal;
}

void MachGuiDbScenario::debriefingLosePicture(const std::string& filename)
{
    pData_->debriefingLosePicture_ = filename;
}

const std::string& MachGuiDbScenario::debriefingLosePicture() const
{
    return pData_->debriefingLosePicture_;
}

void MachGuiDbScenario::hasBeenWon(bool newValue)
{
    pData_->hasBeenWon_ = newValue;
}

bool MachGuiDbScenario::hasBeenWon() const
{
    return pData_->hasBeenWon_;
}

/* End DBSCENAR.CPP *************************************************/
