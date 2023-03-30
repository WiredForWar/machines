/*
 * M A P A R E A . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiMapArea

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_MAPAREA_HPP
#define _MACHGUI_MAPAREA_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"

class MachGuiMapAreaImpl;

class MachGuiMapArea : public GuiDisplayable
// Canonical form revoked
{
public:
    MachGuiMapArea(GuiDisplayable* pParent, const Gui::Box&);
    ~MachGuiMapArea() override;

    void CLASS_INVARIANT;

    void doDisplay() override;

    void controlPanelSliding(bool);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiMapArea& t);

    MachGuiMapArea(const MachGuiMapArea&);
    MachGuiMapArea& operator=(const MachGuiMapArea&);

    void redrawAreaImmediate(const Gui::Box& area);

    MachGuiMapAreaImpl* pImpl_;
};

#endif

/* End MAPAREA.HPP **************************************************/
