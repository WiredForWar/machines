/*
 * P 1 R E M M A N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/p1remman.hpp"

#include "ctl/pvector.hpp"
#include "mathex/point2d.hpp"
#include "machlog/p1handlr.hpp"
#include "machlog/p1mchndl.hpp"
#include "machlog/races.hpp"
#include "machlog/actor.hpp"

using Handlers = ctl_pvector<MachLog1stPersonHandler>;
class MachLogRemoteFirstPersonManagerImpl
{
public:
    Handlers handlers_;
};

MachLogRemoteFirstPersonManager::MachLogRemoteFirstPersonManager()
{
    // Create and initialise the pimple
    pImpl_ = new MachLogRemoteFirstPersonManagerImpl;

    Handlers& handlers_ = pImpl_->handlers_;

    handlers_.reserve(MachPhys::N_RACES);
    for (size_t i = 0; i != MachPhys::N_RACES; ++i)
        handlers_.push_back(nullptr);

    TEST_INVARIANT;
}

MachLogRemoteFirstPersonManager::~MachLogRemoteFirstPersonManager()
{
    TEST_INVARIANT;

    // Delete the handlers
    closeAll();

    delete pImpl_;
}

void MachLogRemoteFirstPersonManager::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachLogRemoteFirstPersonManager& t)
{

    o << "MachLogRemoteFirstPersonManager " << (void*)&t << " start" << std::endl;
    o << "MachLogRemoteFirstPersonManager " << (void*)&t << " end" << std::endl;

    return o;
}

void MachLogRemoteFirstPersonManager::update()
{
    // Update any handlers
    Handlers& handlers_ = pImpl_->handlers_;
    for (size_t i = 0; i != MachPhys::N_RACES; ++i)
    {
        MachLog1stPersonHandler* pHandler = handlers_[i];
        if (pHandler != nullptr)
            pHandler->update();
    }
}

void MachLogRemoteFirstPersonManager::actorOpen(UtlId actorId, MachPhys::Race race)
{
    PRE(MachLogRaces::instance().actorExists(actorId));
    PRE(race < MachPhys::N_RACES);

    // Get the specified actor
    MachLogRaces& races = MachLogRaces::instance();
    MachActor& actor = races.actor(actorId);

    // Create a new handler of the appropriate kind
    Handlers& handlers_ = pImpl_->handlers_;
    ASSERT(handlers_[race] == nullptr, "Remote first person already exists");

    MachLog1stPersonHandler* pHandler = nullptr;
    if (actor.objectIsMachine())
    {
        // Create the handler
        pHandler = new MachLog1stPersonMachineHandler(&actor.asMachine(), MachLog1stPersonHandler::REMOTE);
        handlers_[race] = pHandler;

        // Setup the camera and tracking entity
        pHandler->initialiseCamera(nullptr);
    }

    // Observe the actor in case it gets deleted locally before a close message arrives
    actor.attach(this);
}

void MachLogRemoteFirstPersonManager::actorClose(UtlId actorId, MachPhys::Race race)
{
    PRE(MachLogRaces::instance().actorExists(actorId));
    PRE(race < MachPhys::N_RACES);

    // Get the specified actor
    MachLogRaces& races = MachLogRaces::instance();
    MachActor& actor = races.actor(actorId);

    // Delete the handler
    Handlers& handlers_ = pImpl_->handlers_;
    delete handlers_[race];
    handlers_[race] = nullptr;

    // Stop observing the actor
    actor.detach(this);
}

void MachLogRemoteFirstPersonManager::closeAll()
{
    Handlers& handlers_ = pImpl_->handlers_;

    for (size_t i = 0; i != handlers_.size(); ++i)
    {
        MachLog1stPersonHandler* handler = handlers_[i];
        if (handler)
        {
            // Stop observing the actor
            handler->actor().detach(this);

            // delete the handler
            delete handlers_[i];
            handlers_[i] = nullptr;
        }
    }
}

void MachLogRemoteFirstPersonManager::actorState(
    UtlId actorId,
    MachPhys::Race race,
    const MachPhysFirstPersonStateVector& state)
{
    PRE(MachLogRaces::instance().actorExists(actorId));
    PRE(race < MachPhys::N_RACES);

    // Do the update provided we have a handler
    Handlers& handlers_ = pImpl_->handlers_;
    MachLog1stPersonHandler* handler = handlers_[race];
    if (handler)
        handler->updateState(state);
}

// virtual
bool MachLogRemoteFirstPersonManager::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int)
{
    bool stayObserving = true;

    switch (event)
    {
        case W4dSubject::DELETED:
            {
                // Get the actor via the id
                stayObserving = false;
                UtlId actorId = pSubject->id();
                MachLogRaces& races = MachLogRaces::instance();
                if (races.actorExists(actorId))
                {
                    // get the actor and race
                    MachActor& actor = races.actor(actorId);
                    MachPhys::Race race = actor.race();

                    // hence get the handler
                    Handlers& handlers_ = pImpl_->handlers_;
                    MachLog1stPersonHandler* handler = handlers_[race];
                    if (handler)
                    {
                        delete handler;
                        handlers_[race] = nullptr;
                    }
                }

                break;
            }
    }

    return stayObserving;
}

// virtual
void MachLogRemoteFirstPersonManager::domainDeleted(W4dDomain*)
{
    // Nothing to do
}

/* End P1REMMAN.CPP *************************************************/
