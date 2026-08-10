/*
 * C O R R A L . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACH_CORRAL_HPP
#define _MACH_CORRAL_HPP

#include "gui/Displayable.hpp"
#include "gui/Icon.hpp"
#include "gui/IconSequence.hpp"
#include "gui/ScrollableList.hpp"

#include "machphys/machphys.hpp"

#include "machgui/gui.hpp"
#include "machgui/ui/ScrollArea.hpp"

#include "world4d/Subject/Observer.hpp"

#include "system/PathName.hpp"
#include "ctl/List.hpp"
#include "ctl/PtrVector.hpp"

//////////////////////////////////////////////////////////////////////

class MachCorralIcons;
class MachInGameScreen;
class MachActor;
class MachGuiHealthBar;

// ##NA start//////////////////////////////////////////////////////////
class MachGuiNewCorralIcon : public GuiButtonWithFilledBorder
{
public:
    MachGuiNewCorralIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const GuiBitmap&);

    ~MachGuiNewCorralIcon() override;

    void setBitmap(const GuiBitmap& bitmap);

    static Gui::Box exteriorRelativeBoundary(const GuiBitmap& bitmap, const GuiBorderMetrics& m, const Gui::Coord& rel);

protected:
    void doDisplayInteriorEnabled(const Gui::Coord& absCoord) override;

    GuiBitmap& corralBitmap();

private:
    MachGuiNewCorralIcon(const MachGuiNewCorralIcon&) = delete;
    MachGuiNewCorralIcon& operator=(const MachGuiNewCorralIcon&) = delete;
    bool operator==(const MachGuiNewCorralIcon&) const = delete;

    GuiBitmap bitmap_;
};
// ##NA end////////////////////////////////////////////////////////////

class MachGuiCorralResource
    // cannonical form revoked
    : public MachGuiNewCorralIcon
    , public W4dObserver
{
public:
    MachGuiCorralResource(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        MachActor* pActor,
        MachInGameScreen* pInGameScreen);

    ~MachGuiCorralResource() override;

    static size_t buttonWidth();
    static size_t buttonHeight();

    ///////////////////////////////

    // true iff this observer is to exist in this subject's list of observers
    // following this call. This will typically be implemented using double dispatch.
    // The clientData is of interest only if event == CLIENT_SPECIFIC.Interpretation
    // is client defined.

    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData = 0) override;

    // Informs observer that an observed domain is being deleted.
    // This observer need not call the W4dDomain::detach() method - this
    // will be done automatically.
    void domainDeleted(W4dDomain* pDomain) override;

protected:
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent&) override;

    void doDisplayInteriorEnabled(const Gui::Coord& absCoord) override;

private:
    MachGuiCorralResource(const MachGuiCorralResource&) = delete;
    MachGuiCorralResource& operator=(const MachGuiCorralResource&) = delete;
    bool operator==(const MachGuiCorralResource&) const = delete;

    void updateHealthBar();
    void updateIcon();

    MachCorralIcons* pParent_{};
    MachActor* pActor_{};
    MachGuiHealthBar* pHealthBar_{};
    bool isObservingActor_{};
    MachInGameScreen* pInGameScreen_{};
};

//////////////////////////////////////////////////////////////////////

class MachCorralIcons;

class MachGuiCorral : public MachGuiScrollArea
// cannonical form revoked
{
public:
    MachGuiCorral(GuiDisplayable* pParent, const Gui::Coord&, MachInGameScreen*);
    ~MachGuiCorral() override;

    static size_t reqWidth();
    static size_t reqHeight(MachInGameScreen* pInGameScreen);

    void clear();

    using Actors = ctl_list<MachActor*>;

    const Actors& actors() const;

    // True if pActor is listed as selected
    bool isSelected(MachActor* pActor) const;

    // Add pActor to the list of selected actors and add its icon
    void add(MachActor* pActor);
    // PRE( not isSelected( pActor ) );

    // Add actors to the list of selected actors and add their icons
    void add(const ctl_pvector<MachActor>& actors);

    // Remove pActor from the selected list, and remove its icon
    void remove(MachActor* pActor);
    // PRE( isSelected( pActor ) );

    // Remove actors from the selected list, and remove their icons
    void remove(const ctl_pvector<MachActor>& actors);

protected:
    GuiSimpleScrollableList* createList(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen*) override;

    Actors& actors();

private:
    // Operations revoked
    MachGuiCorral(const MachGuiCorral&) = delete;
    MachGuiCorral& operator=(const MachGuiCorral&) = delete;
    bool operator==(const MachGuiCorral&) const = delete;

    void eraseAllActors();
    MachPhys::Race race() const;
    friend class MachCorralIcons;

    MachCorralIcons* pIcons_;
    Actors actors_;
    MachInGameScreen* pInGameScreen_;
};

/* //////////////////////////////////////////////////////////////// */

class MachCorralIcons : public GuiSimpleScrollableList
// cannonical from revoked
{
public:
    MachCorralIcons(MachGuiCorral* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);
    ~MachCorralIcons() override;

    static size_t reqWidth();
    static size_t reqHeight(MachInGameScreen* pInGameScreen);

    static size_t columns();

    void add(MachActor* pActor);
    void change();

protected:
    void doDisplay() override;

private:
    // Operations revoked
    MachCorralIcons(const MachCorralIcons&) = delete;
    MachCorralIcons& operator=(const MachCorralIcons&) = delete;
    bool operator==(const MachCorralIcons&) const = delete;

    // Data members...
    MachGuiCorral* pCorral_;
    MachInGameScreen* pInGameScreen_;
};

#endif // #ifndef _MACH_CORRAL_HPP

/* End CORRAL.HPP ***************************************************/
