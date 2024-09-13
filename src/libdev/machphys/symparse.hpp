/*
 * S Y M P A R S E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    Symparse

    A collection of static parsing functions for MachPhys objects
*/

#ifndef _SYMPARSE_HPP
#define _SYMPARSE_HPP

#include "machphys/machphys.hpp"

#include "stdlib/string.hpp"

class MachPhysSymbolParser
{
public:
    MachPhysSymbolParser() {};
    ~MachPhysSymbolParser();

    static MachPhys::ConstructionType constructionType(const std::string& token);
    static MachPhys::AggressorSubType aggressorSubType(const std::string&);
    static MachPhys::AdministratorSubType administratorSubType(const std::string&);
    static MachPhys::ConstructorSubType constructorSubType(const std::string&);
    static MachPhys::TechnicianSubType technicianSubType(const std::string&);
    static MachPhys::FactorySubType factorySubType(const std::string&);
    static MachPhys::HardwareLabSubType hardwareLabSubType(const std::string&);
    static MachPhys::MissileEmplacementSubType missileEmplacementSubType(const std::string&);
    static int voidSubType(const std::string& token);

    static int constructionSubType(MachPhys::ConstructionType type, const std::string& subType);

    static size_t parseConstructionLevel(const std::string& token);

private:
    // Operation deliberately revoked
    MachPhysSymbolParser(const MachPhysSymbolParser&);

    // Operation deliberately revoked
    MachPhysSymbolParser& operator=(const MachPhysSymbolParser&);

    // Operation deliberately revoked
    bool operator==(const MachPhysSymbolParser&);
};

#endif

/* End SYMPARSE.HPP *************************************************/
