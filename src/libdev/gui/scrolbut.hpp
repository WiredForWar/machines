/*
 * S C R O L B U T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    GuiScrollButton

    A brief description of the class should go in here
*/

#ifndef _GUI_SCROLBUT_HPP
#define _GUI_SCROLBUT_HPP

#include "base/base.hpp"
#include "gui/listobvr.hpp"
#include "gui/button.hpp"

class GuiScrollButton
    : public GuiListObserver
    , public GuiBitmapButtonWithFilledBorder
// Canonical form revoked
{
public:
    enum ScrollDir
    {
        FOWARD,
        BACKWARD,
        END,
        BEGIN
    };

    GuiScrollButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        GuiSimpleScrollableList* pList,
        ScrollDir dir,
        const GuiFilledBorderColours&,
        const GuiBitmap&);
    GuiScrollButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        GuiSimpleScrollableList* pList,
        ScrollDir dir,
        const GuiBitmap&);
    GuiScrollButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        GuiSimpleScrollableList* pList,
        ScrollDir dir,
        const GuiFilledBorderColours&,
        const GuiBitmap& enabledBitmap,
        const GuiBitmap& disabledBitmap);
    GuiScrollButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        GuiSimpleScrollableList* pList,
        ScrollDir dir,
        const GuiBitmap& enabledBitmap,
        const GuiBitmap& disabledBitmap);

    ~GuiScrollButton() override;

    void CLASS_INVARIANT;

protected:
    void listUpdated() override;

    void doBeReleased(const GuiMouseEvent& rel) override;
    void doBeDepressed(const GuiMouseEvent& rel) override;

    const GuiBitmap& getBitmap() const override;

private:
    friend std::ostream& operator<<(std::ostream& o, const GuiScrollButton& t);

    GuiScrollButton(const GuiScrollButton&);
    GuiScrollButton& operator=(const GuiScrollButton&);

    ScrollDir scrollDir_;

    GuiBitmap enabledBitmap_;
    GuiBitmap disabledBitmap_;
};

#endif

/* End SCROLBUT.HPP *************************************************/
