/*
 * P R O G R E S S . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved.
 */

#ifndef _GUI_PROGRESS_HPP
#define _GUI_PROGRESS_HPP

#include "gui/Displayable.hpp"

/* //////////////////////////////////////////////////////////////// */

class GuiProgressBar : public GuiDisplayable
{
public:
    GuiProgressBar(GuiDisplayable* pParent, const Gui::Box& rel);
    // PRE( pParent != NULL );
    // POST( percentageComplete() == 0 );

    ///////////////////////////////

    using Percentage = double;
    Percentage percentageComplete();
    void percentageComplete(Percentage p);
    // PRE( p >= 0.0 and p <= 100.0 );

    ///////////////////////////////

    // inherited from GuiDisplayable...
    // virtual void display() = 0;

protected:
    ~GuiProgressBar() override;

    // width in pixels for a given percentage
    static unsigned progressWidthInPixels(Percentage progress, unsigned progressCompleteWidth);

private:
    GuiProgressBar(const GuiProgressBar&) = delete;
    bool operator==(const GuiProgressBar&) const = delete;
    GuiProgressBar& operator=(const GuiProgressBar&) = delete;

    Percentage percentageComplete_;
};

/* //////////////////////////////////////////////////////////////// */

class GuiFilledProgressBar : public GuiProgressBar
{
public:
    GuiFilledProgressBar(
        GuiDisplayable* pParent,
        const Gui::Box& rel,
        const GuiColour& backgroundCol,
        const GuiColour& progressCol);
    // PRE( pParent != NULL );

    ~GuiFilledProgressBar() override;

    ///////////////////////////////

    // inherited from GuiDisplayable...
    void doDisplay() override;

private:
    GuiFilledProgressBar(const GuiFilledProgressBar&) = delete;
    bool operator==(const GuiFilledProgressBar&) const = delete;
    GuiFilledProgressBar& operator=(const GuiFilledProgressBar&) = delete;

    ///////////////////////////////

    GuiColour backgroundCol_;
    GuiColour progressCol_;
};

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _GUI_PROGRESS_HPP
