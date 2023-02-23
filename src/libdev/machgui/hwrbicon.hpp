/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachHWResearchBankIcon

    Represents an icon in the hardware research menu queue bank
*/

#ifndef _MACHGUI_HWREBICN_HPP
#define _MACHGUI_HWREBICN_HPP

#include "base/base.hpp"

#include "gui/icon.hpp"
#include "machlog/machlog.hpp"

#include "machphys/machphys.hpp"

// forward refs
class GuiDisplayable;
class MachInGameScreen;
class MachLogResearchItem;
class MachGuiBuildProgressBar;

class MachGuiNewResearchIcon : public GuiButtonWithFilledBorder
{
public:
    MachGuiNewResearchIcon(GuiDisplayable* pParent, const Gui::Coord& rel, const GuiBitmap&);

    ~MachGuiNewResearchIcon() override;

    static Gui::Box exteriorRelativeBoundary(const GuiBitmap& bitmap, const GuiBorderMetrics& m, const Gui::Coord& rel);

protected:
    void doDisplayInteriorEnabled(const Gui::Coord& absCoord) override;

private:
    // Operations revoked
    MachGuiNewResearchIcon(const MachGuiNewResearchIcon&);
    MachGuiNewResearchIcon& operator=(const MachGuiNewResearchIcon&);
    bool operator==(const MachGuiNewResearchIcon&) const;

    GuiBitmap bitmap_;
};

class MachHWResearchBankIcon : public MachGuiNewResearchIcon
// orthodox canonical (revoked)
{
public:
    // ctor.
    MachHWResearchBankIcon(
        GuiDisplayable* pParent,
        MachInGameScreen* pInGameScreen,
        const MachLogResearchItem* pResearchItem,
        MachPhys::Race race);

    // dtor.
    ~MachHWResearchBankIcon() override;

    void CLASS_INVARIANT;

    static size_t reqHeight();
    static size_t reqWidth();

    const MachLogResearchItem* researchItem() const;

    void updateProgress(float percentageComplete);

protected:
    // inherited from GuiButton...
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent&) override;

    void displayCursorPromptText();

private:
    // Operations deliberately revoked
    MachHWResearchBankIcon(const MachHWResearchBankIcon&);
    MachHWResearchBankIcon& operator=(const MachHWResearchBankIcon&);
    bool operator==(const MachHWResearchBankIcon&);

    friend ostream& operator<<(ostream& o, const MachHWResearchBankIcon& t);

    // Data members
    MachGuiBuildProgressBar* pProgressBar_;
    MachInGameScreen* pInGameScreen_;
    const MachLogResearchItem* pResearchItem_;
    bool needsPromptUpdate_;
};

#endif

/* End  *************************************************/
