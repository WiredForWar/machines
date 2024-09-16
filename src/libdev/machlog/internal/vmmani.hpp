/*
 * V M M A N I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachLogVoiceMailManagerImpl

    A brief description of the class should go in here
*/

#ifndef _MACHLOG_VMMANI_HPP
#define _MACHLOG_VMMANI_HPP

#include "base/base.hpp"
#include "stdlib/string.hpp"
#include "ctl/map.hpp"
#include "ctl/pvector.hpp"
#include "machlog/vmman.hpp"
#include "machlog/vmdata.hpp"

#include <memory>

class MachLogVoiceMailManagerImpl
// Canonical form revoked
{
public:
    friend std::ostream& operator<<(std::ostream& o, const MachLogVoiceMailManagerImpl& t);

    MachLogVoiceMailManager::BoolVec voiceMailPlaying_; // a 'true' value means this actor has a voice mail associated
                                                        // with it that is currently playing.
    MachLogVoiceMailManager::MailInfoVector* pAvailableVEMails_;
    MachLogVoiceMailManager::VEmailIDMap registeredIds_;
    std::vector<std::unique_ptr<MachLogVoiceMail>> incomingMailQueue_;
    size_t currentMailMessage_;
    size_t lastMessage_;
    size_t savedMailslots_;
    bool definitionFileRead_;
    bool podMailPlaying_; // ensures pod (actor-independent mails) is only saying one thing at a time
    int noOfMailsPlaying_; // if we keep track of this, we can insist upon a maximum number allowed to play
    bool acceptMailPostings_; // if this is false, no mails sent to most methods will be added to the queue.
};

#define CB_MachLogVoiceMailManager_DEPIMPL()                                                                           \
    CB_DEPIMPL_AUTO(voiceMailPlaying_);                                                                                \
    CB_DEPIMPL_AUTO(pAvailableVEMails_);                                                                               \
    CB_DEPIMPL_AUTO(registeredIds_);                                                                                   \
    CB_DEPIMPL_AUTO(incomingMailQueue_);                                                                               \
    CB_DEPIMPL_AUTO(currentMailMessage_);                                                                              \
    CB_DEPIMPL_AUTO(lastMessage_);                                                                                     \
    CB_DEPIMPL_AUTO(savedMailslots_);                                                                                  \
    CB_DEPIMPL_AUTO(definitionFileRead_);                                                                              \
    CB_DEPIMPL_AUTO(podMailPlaying_);                                                                                  \
    CB_DEPIMPL_AUTO(noOfMailsPlaying_);                                                                                \
                                                                                                                       \
    CB_DEPIMPL_AUTO(acceptMailPostings_);

#endif

/* End VMMANI.HPP ***************************************************/
