/*
 * C T X S A V E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCtxSave

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CTXSAVE_HPP
#define _MACHGUI_CTXSAVE_HPP

#include "base/base.hpp"
#include "machgui/startctx.hpp"
#include "machgui/animatio.hpp"

class MachGuiSingleSelectionListBox;
class MachGuiEditBoxListBoxItem;
class MachGuiDbSavedGame;
class MachGuiMenuButton;

class MachGuiCtxSave : public MachGuiStartupScreenContext
// Canonical form revoked
{
public:
    MachGuiCtxSave(MachGuiStartupScreens* pStartupScreens);
    ~MachGuiCtxSave() override;

    void CLASS_INVARIANT;

    void update() override;

    bool okayToSwitchContext() override;

    void buttonEvent(MachGuiStartupScreens::ButtonEvent) override;

    void selectedSaveGame(MachGuiDbSavedGame*);
    void clearSelectedSaveGame();

    void deleteSavedGame();
    // PRE( pSelectedSaveGame_ );

protected:
    void updateSaveGameList();

    bool saveGame(const string& saveDisplayName);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCtxSave& t);

    MachGuiCtxSave(const MachGuiCtxSave&);
    MachGuiCtxSave& operator=(const MachGuiCtxSave&);

    // Data members...
    MachGuiAnimations animations_;
    MachGuiSingleSelectionListBox* pSaveGameList_;
    MachGuiEditBoxListBoxItem* pNewSaveGameName_;
    MachGuiDbSavedGame* pSelectedSaveGame_;
    MachGuiMenuButton* pDeleteBtn_;
    MachGuiMenuButton* pOkBtn_;
};

#endif

/* End CTXSAVE.HPP **************************************************/
