/*
 * B U F S C B U T . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiBufferScrollButton

    A brief description of the class should go in here
*/

#ifndef _BUFSCBUT_HPP
#define _BUFSCBUT_HPP

#include "base/base.hpp"

#include "gui/icon.hpp"
#include "gui/listobvr.hpp"

// Forward refs
class MachInGameScreen;
class GuiSimpleScrollableList;

class MachGuiBufferScrollButton
    : public GuiListObserver
    , public GuiIcon
// cannonical from revoked
{
public:
    enum ScrollDir
    {
        LEFT,
        RIGHT,
        FOWARD = RIGHT,
        BACKWARD = LEFT
    };

    MachGuiBufferScrollButton(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        GuiSimpleScrollableList* pList,
        ScrollDir dir,
        MachInGameScreen* pInGameScreen);

    ~MachGuiBufferScrollButton() override;

    void CLASS_INVARIANT;

    static size_t width();

protected:
    void listUpdated() override;

    void doBeDepressed(const GuiMouseEvent&) override;
    void doBeReleased(const GuiMouseEvent&) override;

    const GuiBitmap& getBitmap() const override;
    static const GuiBitmap&
    getBitmap(ScrollDir scrollDir, bool canScrollBackward = false, bool canScrollForward = false);

private:
    // Operations revoked
    MachGuiBufferScrollButton(const MachGuiBufferScrollButton&);
    MachGuiBufferScrollButton& operator=(const MachGuiBufferScrollButton&);
    bool operator==(const MachGuiBufferScrollButton&) const;

    friend ostream& operator<<(ostream& o, const MachGuiBufferScrollButton& t);

    // Data members...
    ScrollDir scrollDir_;
    MachInGameScreen* pInGameScreen_;
};

#endif

/* End BUFSCBUT.HPP *************************************************/
