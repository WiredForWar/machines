/*
 * H E A L T H . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _MACHGUI_HEALTH_HPP
#define _MACHGUI_HEALTH_HPP

//////////////////////////////////////////////////////////////////////

#include "gui/displaya.hpp"
#include "machphys/machphys.hpp"

class MachGuiHealthBar : public GuiDisplayable
// cannonical form revoked
{
public:
    MachGuiHealthBar(
        GuiDisplayable* pParent,
        const Gui::Coord& rel,
        unsigned width,
        MachPhys::HitPointUnits maxHits,
        MachPhys::ArmourUnits maxArmour);
    // PRE( maxHits > 0 );
    // PRE( maxArmour > 0 );

    ~MachGuiHealthBar() override;

    MachPhys::HitPointUnits maxHits() const;
    MachPhys::ArmourUnits maxArmour() const;

    void hp(MachPhys::HitPointUnits hits);
    // PRE( hits < maxHits() );

    void armour(MachPhys::ArmourUnits armour);
    // PRE( armour < maxArmour() );

    static size_t healthBarHeight();

    void depress(bool doDepress);

protected:
    void doDisplay() override;

private:
    MachGuiHealthBar(const MachGuiHealthBar&);
    MachGuiHealthBar& operator=(const MachGuiHealthBar&);
    bool operator==(const MachGuiHealthBar&);

    ///////////////////////////////

    static Gui::Colour hpColour(unsigned currentValue, unsigned maxValue);
    static Gui::Colour armourColour(unsigned currentValue, unsigned maxValue);

    MachPhys::HitPointUnits maxHits_;
    MachPhys::ArmourUnits maxArmour_;

    MachPhys::HitPointUnits currentHits_;
    MachPhys::ArmourUnits currentArmour_;

    friend std::ostream& operator<<(std::ostream& o, const MachGuiHealthBar& t);
};

//////////////////////////////////////////////////////////////////////

#endif // #ifndef _MACHGUI_HEALTH_HPP

//////////////////////////////////////////////////////////////////////

/* End HEALTH.HPP ***************************************************/
