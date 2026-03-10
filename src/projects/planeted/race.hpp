/*
 * N A M E S P A C E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

#pragma once

/*
    PedRace

    Namespace for race - related functions
*/

#include "base/base.hpp"
#include "machphys/machphys.hpp"
#include "render/Colour.hpp"
// #include <strfwd.hpp>

using string = std::string;

struct PedRace
{
    static MachPhys::Race parse(const string& raceString);
    static MachPhys::Race next(MachPhys::Race race);
    static RenColour colour(MachPhys::Race race);
    static uint nRaces() { return MachPhys::N_RACES; }
};
