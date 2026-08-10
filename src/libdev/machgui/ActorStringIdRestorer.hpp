
#ifndef _MACHLOG_ACTNAMID_HPP
#define _MACHLOG_ACTNAMID_HPP

#include "machlog/World/MachLog.hpp"
#include "machphys/machphys.hpp"

#include "gui/StringId.hpp"

class MachActor;

class MachLogActorStringIdRestorer
// cannonical form revoked
{
public:
    MachLogActorStringIdRestorer();
    virtual ~MachLogActorStringIdRestorer();

    // Returns string id used to display actors sub-type ( e.g. Lab-Tech, Commanded, Grunt ). Add hardware
    // level to get stringId that describes specific machine type ( e.g. Judas Warlord, Shark )
    static Gui::StringId stringId(const MachActor*);
    static Gui::StringId stringId(MachLog::ObjectType objType, size_t subType, int level);

    // Returns string id used to display name of weapon.
    static Gui::StringId weaponStringId(MachPhys::WeaponCombo wc);

    // To display full id text about a machine the weaponStringId and stringId+hardware level
    // are used ( stringId + hardware level + "WITH" + weaponStringId ). Sometimes this is not
    // correct therefore isExceptionToRule should be called first which will return true if
    // the standard way of displaying full info is not sufficient. isExceptionToRule will return
    // the correct stringId which should not be modified in any way ( i.e. do not add hardware level ).
    static bool isExceptionToRule(
        MachLog::ObjectType objType,
        size_t subType,
        MachPhys::WeaponCombo wc,
        int level,
        Gui::StringId* pStringId);
    static bool isExceptionToRule(const MachActor*, Gui::StringId* pStringId);

    static std::string getActorPromptText(const MachActor*, Gui::StringId actorStrId, Gui::StringId actorWithWeaponStrId);
    static std::string getActorPromptText(
        MachLog::ObjectType objType,
        size_t subType,
        MachPhys::WeaponCombo wc,
        int level,
        Gui::StringId actorStrId,
        Gui::StringId actorWithWeaponStrId);

private:
    static Gui::StringId stringId(MachLog::ObjectType objType, size_t subType);

    MachLogActorStringIdRestorer(const MachLogActorStringIdRestorer&) = delete;
    MachLogActorStringIdRestorer& operator=(const MachLogActorStringIdRestorer&) = delete;
    bool operator==(const MachLogActorStringIdRestorer&) = delete;

    // Data members...
    Gui::StringId stringId_{};
};

/* //////////////////////////////////////////////////////////////// */

#endif // #ifndef _MACHLOG_RESTORER_HPP

/* End RESTORER.HPP *************************************************/
