/*
 * D B S C E N A R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiDbScenario

    Represents a scenario in a campaign, skirmish or multi-player game.
*/

#ifndef _MACHGUI_DBSCENAR_HPP
#define _MACHGUI_DBSCENAR_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "machphys/machphys.hpp"
#include "machgui/dbelemen.hpp"

// forward refs
class MachGuiDbIScenario;
class MachGuiDbPlanet;

class MachGuiDbScenario : public MachGuiDbElement
// Canonical form revoked
{
public:
    // The scenario is known by identifier name which is used to construct the scenario filename.
    // The relative path for the planet files is planetFile.
    // The id of the std::string used in the menu list boxes is menuStringId.
    //  MachGuiDbScenario( const std::string& name, const std::string& planetFile, uint menuStringId );
    MachGuiDbScenario(const std::string& name, const std::string& planetFile, uint menuStringId, uint maxPlayers);

    ~MachGuiDbScenario() override;

    // set/get the logical planet on which the scenario occurs
    void planet(MachGuiDbPlanet* pPlanet);
    MachGuiDbPlanet& planet() const;

    // set/get the planet file name for the scenario
    void planetFile(const std::string& filename);
    const std::string& planetFile() const;

    // set/get the flic or bmp filename displayed in the campaign menu
    // std::string is empty if undefined.
    void campaignPicture(const std::string& filename);
    const std::string& campaignPicture() const;

    // set/get the flic/bmp filename displayed in the briefing menu
    // std::string is empty if undefined.
    void briefingPicture(const std::string& filename);
    const std::string& briefingPicture() const;

    // set/get the flic/bmp filename displayed in the debriefing menu
    // std::string is empty if undefined.
    void debriefingPicture(const std::string& filename);
    const std::string& debriefingPicture() const;
    void debriefingLosePicture(const std::string& filename);
    const std::string& debriefingLosePicture() const;

    // set/get the fullscreen flic to be displayed upon starting a scenario
    // std::string is empty if undefined.
    void entryFlic(const std::string& filename);
    const std::string& entryFlic() const;

    // set/get the fullscreen flic to be displayed upon starting a scenario
    // std::string is empty if undefined.
    void winFlic(const std::string& filename);
    const std::string& winFlic() const;

    // set/get the fullscreen flic to be displayed upon starting a scenario
    // std::string is empty if undefined.
    void loseFlic(const std::string& filename);
    const std::string& loseFlic() const;

    // get the maximum number of players that play a scenario
    uint maxPlayers() const;
    void CLASS_INVARIANT;

    // Number of races for which we need save a list of the surviving machines
    // on successful completion
    uint nRacesToHaveSurvivingMachinesSaved() const;

    // The index'th such race
    MachPhys::Race raceToHaveSurvivingMachinesSaved(uint index);
    // PRE( index < nRacesToHaveSurvivingMachinesSaved() );

    // Add another race to have its machines saved
    void raceToHaveSurvivingMachinesSaved(MachPhys::Race);

    // Music track to be played
    uint musicTrack() const;
    void musicTrack(uint track);

    // Check if this scenario is one of the training levels. This is implemented via a bit of a hack but
    // with only a few days to go...
    bool isTrainingScenario() const;

    // Indicate that scenario has been won by someone.
    void hasBeenWon(bool);
    bool hasBeenWon() const;

    PER_MEMBER_PERSISTENT_VIRTUAL(MachGuiDbScenario)
    PER_FRIEND_READ_WRITE(MachGuiDbScenario)

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiDbScenario& t);

    MachGuiDbScenario(const MachGuiDbScenario&);
    MachGuiDbScenario& operator=(const MachGuiDbScenario&);

    void initialise(const std::string& scenarioName, const std::string& planetName);

    // data members
    MachGuiDbIScenario* pData_; // implementation data object
};

PER_DECLARE_PERSISTENT(MachGuiDbScenario);

#endif

/* End DBSCENAR.HPP *************************************************/
