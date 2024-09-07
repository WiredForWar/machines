/*
 * C T X J O I N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxJoin

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXJOIN_HPP
#define _MACHGUI_CTXJOIN_HPP

#include "machgui/GameMenuContext.hpp"

class MachGuiSingleSelectionListBox;
class MachGuiNewGameName;
class MachGuiNetworkProtocolMode;
class MachGuiMenuButton;
class NetSessionInfo;

class MachGuiCtxJoin : public MachGui::GameMenuContext
// Canonical form revoked
{
public:
    MachGuiCtxJoin(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxJoin() override;

    void buttonEvent(MachGui::ButtonEvent) override;

    bool okayToSwitchContext() override;

    void update() override;

    void changeFocus();

    bool validNetworkSettings(bool isHost);

    void editingGameName(bool);
    void joinGameSelected(bool);

    void onNewGameItemSelected();
    void onNetSessionSelected(const NetSessionInfo& info);

    void CLASS_INVARIANT;

protected:
    void showGames();
    std::size_t numGamesInList() const;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxJoin& t);

    MachGuiCtxJoin(const MachGuiCtxJoin&);
    MachGuiCtxJoin& operator=(const MachGuiCtxJoin&);

    // Data members...
    MachGuiSingleSelectionListBox* pGamesList_;
    MachGuiNewGameName* pNewGameName_;
    MachGuiNetworkProtocolMode* pNetworkProtocol_{};
    bool editingGameName_{};
    bool joinGameSelected_{};
    MachGuiMenuButton* pJoinBtn_;
    MachGuiMenuButton* pCreateBtn_;
    MachGuiMenuButton* pShowGamesBtn_;
};

#endif

/* End CTXJOIN.HPP **************************************************/
