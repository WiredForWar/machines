/*
 * C T X S E T N G . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxSettings

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSETNG_HPP
#define _MACHGUI_CTXSETNG_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiScrollableText;
class MachGuiSingleSelectionListBox;
class MachGuiDbSystem;
class MachGuiDbPlanet;
class MachGuiDbScenario;
class GuiSimpleScrollableList;
class MachGuiDropDownListBoxCreator;

class MachGuiCtxSettings : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxSettings(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxSettings() override;

    void CLASS_INVARIANT;

    void update() override;

    void updateTerrainTypeList(MachGuiDbSystem&);
    void updateScenarioList(MachGuiDbPlanet&);
    void updateSelectedScenario(MachGuiDbScenario&);

    bool okayToSwitchContext() override;

protected:
    void addSetting(MachGuiDropDownListBoxCreator*& pCreator, uint labelStrId, int numStrs, ...);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxSettings& t);

    MachGuiCtxSettings(const MachGuiCtxSettings&);
    MachGuiCtxSettings& operator=(const MachGuiCtxSettings&);

    void updateMapSizeList();
    void initSettings();

    // Data members...
    MachGuiScrollableText* pScenarioDesc_;
    MachGuiSingleSelectionListBox* pMapSizeList_;
    MachGuiSingleSelectionListBox* pTerrainTypeList_;
    MachGuiSingleSelectionListBox* pScenarioList_;
    MachGuiDbScenario* pSelectedScenario_;
    GuiSimpleScrollableList* pSettingsList_;
    MachGuiDropDownListBoxCreator* pFogOfWarSelector_;
    MachGuiDropDownListBoxCreator* pResourcesSelector_;
    MachGuiDropDownListBoxCreator* pStartingResourcesSelector_;
    MachGuiDropDownListBoxCreator* pStartingPosSelector_;
    MachGuiDropDownListBoxCreator* pVictorySelector_;
    MachGuiDropDownListBoxCreator* pTechLevelSelector_;
    MachGuiDropDownListBoxCreator* pAlliancesSelector_;
    MachGuiDropDownListBoxCreator* pDisableFirstPersonSelector_;
};

#endif

/* End CTXSETNG.HPP *************************************************/
