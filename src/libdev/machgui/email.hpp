/*
 * E M A I L . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _MACH_EMAIL_HPP
#define _MACH_EMAIL_HPP

#include "gui/gui.hpp"
#include "gui/icon.hpp"
#include "gui/root.hpp"
#include "gui/displaya.hpp"
#include "machgui/gui.hpp"

class MachInGameScreen;

class MachEmailWindow : public GuiDisplayable
{
public:
    ~MachEmailWindow() override {};

protected:
    // inherited from GuiDisplayable...
    void doDisplay() override {};
};

class MachMenuBar : public GuiDisplayable
{
public:
    MachMenuBar(MachInGameScreen* pParent, const Gui::Coord& rel);

    ~MachMenuBar() override {};

protected:
    // inherited from GuiDisplayable...
    void doDisplay() override {};
};

class MachEmailArrivalBar : public GuiDisplayable
{
public:
    MachEmailArrivalBar(MachInGameScreen* pParent, const Gui::Coord& rel);

    ~MachEmailArrivalBar() override {};

protected:
    // inherited from GuiDisplayable...
    void doDisplay() override {};
};

/* //////////////////////////////////////////////////////////////// */

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _MACH_INGAME_HPP

/* End EMAIL.HPP ***************************************************/