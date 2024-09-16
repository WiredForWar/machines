/*
 * V M M A N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVoiceMailManager

    A singleton managing the queue of voice mails in the game
*/

#ifndef _MACHLOG_VMMAN_HPP
#define _MACHLOG_VMMAN_HPP

#include "base/base.hpp"
#include "machlog/vmdata.hpp"
#include "machphys/machphys.hpp"
#include "utility/id.hpp"

#include <string>
#include <vector>

class MexPoint3d;
class MachLogVoiceMail;
class MachLogVoiceMailInfo;
class MachLogVoiceMailManagerImpl;
template <class X, class Y, class Z> class ctl_map;
template <class T> class ctl_pvector;
template <class X> class ctl_vector;
// template < class T > class std::less;

// Warnings about foward references of template classes
#ifndef CB_NOWARNINGS // Are Charybdis warnings turned on?
#ifndef _STD_STRING_HPP
#pragma message(__FILE__ "(13): Warning : std::less< std::string > foward referenced")
#pragma message(__FILE__ "(13): Warning : ctl_map< std::string, VoiceMailID, std::less< std::string > > foward referenced")
#endif
#endif

class MachLogVoiceMailManager
// Canonical form revoked
{
public:
    using BoolVec = std::vector<bool>;

    //  Singleton class
    static MachLogVoiceMailManager& instance();
    ~MachLogVoiceMailManager();

    // Update voicemail Queue and play all new mails
    void update();

    // flush everything out of the voicemail queue
    void clearMailQueue();

    // At default setting decCurrentMail will play the most recent saved mail,
    // and incCurrentMail will return false, otherwise the current mail
    // is where it was left at by the last call to inc/dec.
    // The current mail is set to default when a new mail of type
    // VM_FULL_FUNCTION is saved.
    // Increment current mail - move to next most recent mail and play
    // returns false and doesn't play if at newest mail
    bool incCurrentMail();
    // Decrement current mail - move to next less recent mail and play
    // returns false and doesn't play if at oldest mail
    bool decCurrentMail();

    // Post verbal mail onto queue
    bool postNewMail(VoiceMailID id, MachPhys::Race targetRace);
    bool postNewMail(VoiceMailID, UtlId actorId, MachPhys::Race targetRace);
    bool postNewMail(VoiceMailID, MexPoint3d position, MachPhys::Race targetRace);

    // special case posting that replaces any currently playing mail with static, and boots any other
    // outstanding mails for that actor off the queue
    void postDeathMail(UtlId actorId, MachPhys::Race targetRace);

    // Set total number of possible stored mails
    void nMailSlots(size_t newNoOfSlots);
    // Get total number of possible stored mails
    size_t nMailSlots() const;

    // Is an actor id associated with current mail
    bool currentMailHasActorId() const;
    // Is an actor id associated with current mail
    UtlId currentMailActorId() const;

    // Has the current mail an associated position
    bool currentMailHasPosition() const;
    // Return the local transform of the actor associated with a mail
    const MexPoint3d currentMailPosition() const;

    using VEmailIDMap = ctl_map<std::string, VoiceMailID, std::less<std::string>>;
    const VEmailIDMap& veMailIDMap() const;

    static bool voiceMailsActivated();

    // if this is set to false, any posted mail is ignored (not added to the queue)
    void acceptMailPostings(bool status);
    bool acceptMailPostings() const;

    void CLASS_INVARIANT;

    using VEmailTypeMap = ctl_map<std::string, VoiceMailType, std::less<std::string>>;
    using MailInfoVector = ctl_pvector<MachLogVoiceMailInfo>;
    using MailVector = ctl_pvector<MachLogVoiceMail>;

private:
    // Read voice mail definition file and register the available VID's
    void registerVoiceMailIDs();

    void assignMappingGroup1();
    void assignMappingGroup2();

    bool canPostMailForRace(MachPhys::Race targetRace) const;
    void queueMail(MachLogVoiceMail* pNewMail);

    friend std::ostream& operator<<(std::ostream& o, const MachLogVoiceMailManager& t);

    MachLogVoiceMailManager(const MachLogVoiceMailManager&);
    MachLogVoiceMailManager& operator=(const MachLogVoiceMailManager&);

    MachLogVoiceMailManager();

    // typedef ctl_map< string, VoiceMailType, std::less< string > > VEmailTypeMap;
    // typedef ctl_pvector< MachLogVoiceMailInfo > MailInfoVector;
    // typedef ctl_pvector< MachLogVoiceMail > MailVector;

    MachLogVoiceMailManagerImpl* pImpl_;
};

#endif

/* End VMMAN.HPP ****************************************************/
