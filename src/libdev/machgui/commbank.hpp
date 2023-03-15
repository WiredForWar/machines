/*
 * C O M M B A N K . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACH_COMMBANK_HPP
#define _MACH_COMMBANK_HPP

#include "gui/icon.hpp"
#include "gui/iconseq.hpp"

#include "machgui/gui.hpp"

/* //////////////////////////////////////////////////////////////// */

class MachInGameScreen;
class MachGuiCommand;

class MachCommandIcons : public GuiScrollableIconSequence
// cannonical from revoked
{
public:
    MachCommandIcons(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);

    ~MachCommandIcons() override;

    void change();

    static size_t reqWidth();
    static size_t reqHeight();

    static size_t horizontalSpacing();

protected:
    virtual const Coords& coords() const;
    void doDisplay() override;

private:
    MachCommandIcons(const MachCommandIcons&);
    MachCommandIcons& operator=(const MachCommandIcons&);
    bool operator==(const MachCommandIcons&) const;

    static const Coords& staticCoords();

    ///////////////////////////////

    unsigned offset_;
    MachInGameScreen* pInGameScreen_;
};

class MachSmallCommandIcons : public GuiScrollableIconSequence
// cannonical from revoked
{
public:
    MachSmallCommandIcons(GuiDisplayable* pParent, const Gui::Coord& rel, MachInGameScreen* pInGameScreen);

    ~MachSmallCommandIcons() override;

    void change();

    static size_t reqWidth();
    static size_t reqHeight();

protected:
    virtual const Coords& coords() const;
    void doDisplay() override;

private:
    MachSmallCommandIcons(const MachSmallCommandIcons&);
    MachSmallCommandIcons& operator=(const MachSmallCommandIcons&);
    bool operator==(const MachSmallCommandIcons&) const;

    static const Coords& staticCoords();

    ///////////////////////////////

    unsigned offset_;
    MachInGameScreen* pInGameScreen_;
};

/* //////////////////////////////////////////////////////////////// */

class MachCommandIcon : public GuiBitmapButtonWithFilledBorder
// cannonical form revoked
{
public:
    MachCommandIcon(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        const MachGuiCommand& command,
        MachInGameScreen* pInGameScreen);

    ~MachCommandIcon() override;

    ///////////////////////////////////////////////////////
    // Inherited from GuiDisplayable
    void doHandleMouseEnterEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

    ///////////////////////////////////////////////////////

    static size_t reqWidth();
    static size_t reqHeight();

    const MachGuiCommand* pCommand() const;

protected:
    MachInGameScreen& inGameScreen();
    const MachInGameScreen& inGameScreen() const;

    // Inherited from GuiButton
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;

    // Inherited from GuiDisplayable
    bool isEligableForVisibility() const override;

private:
    MachCommandIcon(const MachCommandIcon&);
    MachCommandIcon& operator=(const MachCommandIcon&);
    bool operator==(const MachCommandIcon&) const;

    // Helper function used in initialisation list
    std::pair<std::string, std::string> bitmapPaths(const MachGuiCommand& command) const;

    MachInGameScreen* pInGameScreen_;
    const MachGuiCommand* pCommand_; // The command instance to be activated on clicking the icon
};

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _MACH_COMMBANK_HPP

/* End COMMBANK.HPP *************************************************/
