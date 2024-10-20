/*
 * C T X S C E N R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxScenario

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSCENR_HPP
#define _MACHGUI_CTXSCENR_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiSingleSelectionListBox;
class MachGuiDbSystem;
class MachGuiDbPlanet;
class MachGuiDbScenario;
class MachGuiScrollableText;
class GuiImage;

class MachGuiCtxScenario : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxScenario(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxScenario() override;

    void CLASS_INVARIANT;

    void update() override;

    void updatePlanetList(MachGuiDbSystem&);
    void updateScenarioList(MachGuiDbPlanet&);
    void updateSelectedScenario(MachGuiDbScenario&);

    void updateDisplayedInfo(const std::string& text, SysPathName animation);

    bool okayToSwitchContext() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxScenario& t);

    MachGuiCtxScenario(const MachGuiCtxScenario&);
    MachGuiCtxScenario& operator=(const MachGuiCtxScenario&);

    void updateSystemList();

    void workOutWhichScenarioToDefaultTo();

    // Data members...
    MachGuiSingleSelectionListBox* pSystemList_;
    MachGuiSingleSelectionListBox* pPlanetList_;
    MachGuiSingleSelectionListBox* pScenarioList_;
    MachGuiScrollableText* pTextInfo_;
    GuiImage* pImage_{};
    MachGuiDbScenario* pDefaultScenario_;
};

#endif

/* End CTXSCENR.HPP *************************************************/
