/*
 * C T X S K I R M . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxInSkirmish

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSKIRM_HPP
#define _MACHGUI_CTXSKIRM_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiScrollableText;
class MachGuiSingleSelectionListBox;
class MachGuiDbSystem;
class MachGuiDbPlanet;
class MachGuiDbScenario;
class GuiSimpleScrollableList;
class MachGuiDropDownListBoxCreator;
class MachGuiText;

class MachGuiCtxSkirmish : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxSkirmish(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxSkirmish() override;

    void CLASS_INVARIANT;

    void update() override;

    void updateTerrainTypeList(MachGuiDbSystem&);
    void updateScenarioList(MachGuiDbPlanet&);
    void updateSelectedScenario(MachGuiDbScenario&);

    bool okayToSwitchContext() override;
    void buttonEvent(MachGui::ButtonEvent) override;

    void autoLoadGame();

    static int& chosenNumPlayers();

protected:
    MachGuiText* addSetting(bool numPlayers, MachGuiDropDownListBoxCreator*&, uint labelStrId, int numStrs, ...);

    void updateMaxPlayersList(MachGuiDbScenario&);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxSkirmish& t);

    MachGuiCtxSkirmish(const MachGuiCtxSkirmish&);
    MachGuiCtxSkirmish& operator=(const MachGuiCtxSkirmish&);

    void updateMapSizeList();
    void initSettings();

    // Data members...
    MachGuiScrollableText* pScenarioDesc_;
    MachGuiSingleSelectionListBox* pMapSizeList_;
    MachGuiSingleSelectionListBox* pTerrainTypeList_;
    MachGuiSingleSelectionListBox* pScenarioList_;
    GuiSimpleScrollableList* pSettingsList_;
    MachGuiDropDownListBoxCreator* pFogOfWarSelector_;
    MachGuiDropDownListBoxCreator* pResourcesSelector_;
    MachGuiDropDownListBoxCreator* pStartingResourcesSelector_;
    MachGuiDropDownListBoxCreator* pStartingPosSelector_;
    MachGuiDropDownListBoxCreator* pVictorySelector_;
    MachGuiDropDownListBoxCreator* pNumPlayersSelector_;
    MachGuiDropDownListBoxCreator* pTechLevelSelector_;
    MachGuiDropDownListBoxCreator* pColourSelector_;
    MachGuiText* pNumPlayersText_;
    bool autoLoadGame_;
};

#endif

/* End CTXSKIRM.HPP *************************************************/
