/*
 * C T X C A M P . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxCampaign

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXCAMP_HPP
#define _MACHGUI_CTXCAMP_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiSingleSelectionListBox;
class MachGuiEditBoxListBoxItem;
class MachGuiDbPlayer;
class MachGuiMenuText;

class MachGuiCtxCampaign : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxCampaign(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxCampaign() override;

    void CLASS_INVARIANT;

    void update() override;

    bool okayToSwitchContext() override;

    void buttonEvent(MachGui::ButtonEvent) override;

    void selectedPlayer(MachGuiDbPlayer*);
    void clearSelectedPlayer();

    void deletePlayer();
    // PRE( pSelectedPlayer_ );

protected:
    void updatePlayersList();

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxCampaign& t);

    MachGuiCtxCampaign(const MachGuiCtxCampaign&);
    MachGuiCtxCampaign& operator=(const MachGuiCtxCampaign&);

    void displayCurrentStatus();
    // Data members...
    MachGuiSingleSelectionListBox* pPlayersList_;
    MachGuiEditBoxListBoxItem* pNewPlayerName_;
    MachGuiDbPlayer* pSelectedPlayer_;
    MachGuiMenuText* pScenarioText_;
    MachGuiMenuText* pLastScenarioText_;
    MachGuiMenuText* pScoreText_;
    MachGuiMenuText* pLastScoreText_;
    MachGuiMenuText* pNumScenariosPlayedText_;
    MachGuiMenuText* pNumScenariosText_;
    MachGuiMenuText* pTotalScoreHeadingText_;
    MachGuiMenuText* pTotalScoreText_;
};

#endif

/* End CTXCAMP.HPP **************************************************/
