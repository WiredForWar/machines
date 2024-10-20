/*
 * C T X R E A D Y . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxImReady

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXREADY_HPP
#define _MACHGUI_CTXREADY_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiChatWindow;
class MachGuiSingleLineEditBox;
class GuiSimpleScrollableList;
class MachGuiPlayerListItem;
class MachGuiMenuButton;
class MachGuiImReadyButton;
class MachGuiStartupData;
class DevTimer;

class MachGuiCtxImReady : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxImReady(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxImReady() override;

    bool doHandleKeyEvent(const GuiKeyEvent&) override;

    void update() override;

    void createPlayerList();
    void updatePlayerList();

    void updateGameSettings();

    void CLASS_INVARIANT;

protected:
    void updateStartAndReadyButtons();
    void displaySystemMessages();

    void buttonEvent(MachGui::ButtonEvent) override;

    bool okayToSwitchContext() override;

    MachGuiStartupData& startupData();
    // PRE( pStartupScreens_ );
    // PRE( pStartupScreens_->startupData() );

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxImReady& t);

    MachGuiCtxImReady(const MachGuiCtxImReady&);
    MachGuiCtxImReady& operator=(const MachGuiCtxImReady&);

    MachGuiChatWindow* pChatWindow_;
    MachGuiSingleLineEditBox* pSingleLineEditBox_;
    GuiSimpleScrollableList* pPlayerList_;
    MachGuiPlayerListItem* pPlayerListItem_[4];
    MachGuiImReadyButton* pImReadyButton_;
    MachGuiMenuButton* pStartButton_;
    MachGuiMenuButton* pSettingsButton_;
    GuiSimpleScrollableList* pReadOnlySettings_;
    DevTimer* pCreationTimer_;
    DevTimer* pHasCDTransmissionTimer_;
};

#endif

/* End CTXREADY.HPP *************************************************/
