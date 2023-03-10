/*
 * P R O D B A N K . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachProductionBank

    Contains the displayables in the Production Menu representing the factory's
    current production queue state.
*/

#ifndef _MACHGUI_PRODBANK_HPP
#define _MACHGUI_PRODBANK_HPP

#include "base/base.hpp"
#include "world4d/observer.hpp"
#include "gui/displaya.hpp"

// Forward refs
class MachProductionIcons;
class MachLogFactory;
class MachInGameScreen;
class GuiFilledProgressBar;
class MachGuiBufferScrollButton;

// orthodox canonical (revoked)
class MachProductionBank
    : public GuiDisplayable
    , public W4dObserver
{
public:
    // ctor. Owned by pParent, with area relativeBoundary.
    // The factory being edited is pFactory.
    // The gui root is pInGameScreen.
    MachProductionBank(
        GuiDisplayable* pParent,
        const Gui::Boundary& relativeBoundary,
        MachLogFactory* pFactory,
        MachInGameScreen* pInGameScreen);

    // dtor
    ~MachProductionBank() override;

    // The required height for the bank
    static int requiredHeight();

    // Update the queue icons
    void updateQueueIcons();

    // Update progress bars
    void updateProgress();

    // inherited from GuiDisplayable...
    void doDisplay() override;

    // The bank's factory
    MachLogFactory& factory();

    ////////////////////////////////////////////////////////////////////////////////
    // Inherited from W4dObserver

    // true iff this observer is to exist in this subject's list of observers
    // following this call. This will typically be implemented using double dispatch.
    // The clientData is of interest only if event == CLIENT_SPECIFIC.Interpretation
    // is client defined.
    bool beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int clientData) override;

    // Informs observer that an observed domain is being deleted.
    // This observer need not call the W4dDomain::detach() method - this
    // will be done automatically.
    void domainDeleted(W4dDomain* pDomain) override;

    ////////////////////////////////////////////////////////////////////////////////
    static size_t width();
    static size_t height();

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachProductionBank& t);

private:
    // Operation deliberately revoked
    MachProductionBank(const MachProductionBank&);
    MachProductionBank& operator=(const MachProductionBank&);
    bool operator==(const MachProductionBank&);

    // Data members
    MachLogFactory* pFactory_; // The factory being edited
    MachProductionIcons* pIcons_; // The icon sequence depicting the queue machine icons
    GuiFilledProgressBar* pProgressBar_; // The progress of the current build icons
    bool observingFactory_; // True while the observer relation on the factory exists
    MachGuiBufferScrollButton* pScrollLeft_;
    MachGuiBufferScrollButton* pScrollRight_;
};

#endif

/* End PRODBANK.HPP *************************************************/
