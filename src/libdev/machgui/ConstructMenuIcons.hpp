/*
 * C O N S I C N S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachConstructMenuIcons

    The construct icons displayed during the selection of a specific construction type.
*/

#ifndef _MACHGUI_CONSICNS_HPP
#define _MACHGUI_CONSICNS_HPP

#include "base/base.hpp"
#include "gui/ScrollableList.hpp"
#include "gui/gui.hpp"

#include "machlog/World/Notifiable.hpp"

// Forward refs
class MachInGameScreen;
class MachLogConstructionTree;

// orthodox canonical (revoked)
class MachConstructMenuIcons
    : public GuiSimpleScrollableList
    , public MachLogNotifiable
{
public:
    // ctor. Located in pParent, covering area.
    // In game screen passed.
    MachConstructMenuIcons(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);

    // dtor.
    ~MachConstructMenuIcons() override;

    void refreshConstructIcons();

    void notifiableBeNotified() override;

    void CLASS_INVARIANT;

    static size_t reqWidth();
    static size_t reqHeight(MachInGameScreen* pInGameScreen);

protected:
    void doDisplay() override;

private:
    // Operations deliberately revoked
    MachConstructMenuIcons(const MachConstructMenuIcons&) = delete;
    MachConstructMenuIcons& operator=(const MachConstructMenuIcons&) = delete;
    bool operator==(const MachConstructMenuIcons&) = delete;

    friend std::ostream& operator<<(std::ostream& o, const MachConstructMenuIcons& t);

    // Data members...
    MachInGameScreen* pInGameScreen_;
    MachLogConstructionTree& consTree_;
};

#endif

/* End CONSICNS.HPP *************************************************/
