/*
 * B M U T E X T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiBmuText

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_BMUTEXT_HPP
#define _MACHGUI_BMUTEXT_HPP

#include "base/base.hpp"
#include "gui/Displayable.hpp"
#include "gui/Icon.hpp"
#include "system/PathName.hpp"

class MachContinentMap;
class MachInGameScreen;
class GuiMouseEvent;
using SysPathNames = std::pair<SysPathName, SysPathName>;

class MachGuiBmuText : public GuiDisplayable
// Canonical form revoked
{
public:
    MachGuiBmuText(GuiDisplayable* pParent, const Gui::Coord& relPos);
    ~MachGuiBmuText() override;

    void CLASS_INVARIANT;

    void refresh();

protected:
    void doDisplay() override;

    void doHandleMouseClickEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent& rel) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& rel) override;
    void doHandleContainsMouseEvent(const GuiMouseEvent& rel) override;

private:
    MachGuiBmuText(const MachGuiBmuText&) = delete;
    MachGuiBmuText& operator=(const MachGuiBmuText&) = delete;
    bool operator==(const MachGuiBmuText&) = delete;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiBmuText& t);

    size_t curValue_;
    size_t maxValue_;
};

class MachGuiBmuButton : public GuiBitmapButtonWithFilledBorder
// cannonical from revoked
{
public:
    MachGuiBmuButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const GuiBitmap& bitmap,
        MachGuiBmuText* pBmuText,
        MachContinentMap* pContinentMap,
        MachInGameScreen* pInGameScreen);
    ~MachGuiBmuButton() override;

protected:
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

private:
    // Operations revoked
    MachGuiBmuButton(const MachGuiBmuButton&) = delete;
    MachGuiBmuButton& operator=(const MachGuiBmuButton&) = delete;
    bool operator==(const MachGuiBmuButton&) const = delete;

    // Data members
    MachGuiBmuText* pBmuText_;
    MachContinentMap* pContinentMap_;
    MachInGameScreen* pInGameScreen_;
};

#endif

/* End BMUTEXT.HPP **************************************************/
