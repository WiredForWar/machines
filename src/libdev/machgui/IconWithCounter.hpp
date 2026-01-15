/*
 * G U I C T R L . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACH_GUICTRL_HPP
#define _MACH_GUICTRL_HPP

#include "gui/displaya.hpp"
#include "gui/icon.hpp"
#include "machgui/gui.hpp"

class MachInGameScreen;

class MachGuiIconWithCounter : public GuiBitmapButtonWithFilledBorder
// cannonical from revoked
{
public:
    MachGuiIconWithCounter(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const GuiBitmap& bitmap,
        MachInGameScreen* pInGameScreen);
    ~MachGuiIconWithCounter() override;

    void setCounterValue(size_t value);

protected:
    void doDisplayInteriorEnabled(const Gui::Coord& abs) override;

    std::string getRootDirectory() const;

    MachInGameScreen* pInGameScreen_ = nullptr;

private:
    MachGuiIconWithCounter(const MachGuiIconWithCounter&);
    MachGuiIconWithCounter& operator=(const MachGuiIconWithCounter&);
    bool operator==(const MachGuiIconWithCounter&) const;

    size_t currentValue_ = 0;
};

class MachMachinesIcon : public MachGuiIconWithCounter
// cannonical from revoked
{
public:
    MachMachinesIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);
    ~MachMachinesIcon() override;

    void refresh();

    // Make the button pop up without calling the doBeRelease
    // functionality ( doBeReleased changes control panel context ).
    void forceUp();

    // Change button bitmap to match race coloue
    void loadGame();

protected:
    // inherited from GuiIcon...
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

private:
    MachMachinesIcon(const MachMachinesIcon&);
    MachMachinesIcon& operator=(const MachMachinesIcon&);
    bool operator==(const MachMachinesIcon&) const;

    // Data members...
    bool forcingUp_;
};

class MachConstructionsIcon : public MachGuiIconWithCounter
// cannonical from revoked
{
public:
    MachConstructionsIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);
    ~MachConstructionsIcon() override;

    void refresh();

    // Make the button pop up without calling the doBeRelease
    // functionality ( doBeReleased changes control panel context ).
    void forceUp();

    // Change button bitmap to match race coloue
    void loadGame();

protected:
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

private:
    MachConstructionsIcon(const MachConstructionsIcon&);
    MachConstructionsIcon& operator=(const MachConstructionsIcon&);
    bool operator==(const MachConstructionsIcon&) const;

    // Data members...
    bool forcingUp_;
};

/* //////////////////////////////////////////////////////////////// */

class MachSquadronIcon : public MachGuiIconWithCounter
// cannonical from revoked
{
public:
    MachSquadronIcon(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);
    ~MachSquadronIcon() override;

    void update();

    // Change button bitmap to match race coloue
    void loadGame();

protected:
    // inherited from GuiIcon...
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

private:
    MachSquadronIcon(const MachSquadronIcon&);
    MachSquadronIcon& operator=(const MachSquadronIcon&);
    bool operator==(const MachSquadronIcon&) const;

    MachInGameScreen* pInGameScreen_;
};

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _MACH_GUICTRL_HPP

/* End GUICTRL.HPP **************************************************/