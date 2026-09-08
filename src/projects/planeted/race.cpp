/*
 * R A C E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#include "planeted/race.hpp"

MachPhys::Race PedRace::parse(const string& race)
{
    // return race based on text string passed in
    return MachPhys::toRace(race).value_or(MachPhys::N_RACES);
}

MachPhys::Race PedRace::next(MachPhys::Race race)
{
    return static_cast<MachPhys::Race>((race + 1) % MachPhys::N_RACES);
}

RenColour PedRace::colour(MachPhys::Race race)
{
    PRE(race < MachPhys::N_RACES);

    static const RenColour colours[MachPhys::N_RACES] = {
        RenColour::red(),
        RenColour::blue(),
        RenColour::green(),
        RenColour::yellow(),
        RenColour(0.5, 0.5, 0.5),
    };

    return colours[race];
}
