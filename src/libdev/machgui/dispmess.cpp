/*
 * D I S P M E S S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/dispmess.hpp"
#include "sim/manager.hpp"

// static
MachGuiMessageDisplay& MachGuiMessageDisplay::instance()
{
    static MachGuiMessageDisplay instance_;
    return instance_;
}

MachGuiMessageDisplay::MachGuiMessageDisplay()
{

    TEST_INVARIANT;
}

MachGuiMessageDisplay::~MachGuiMessageDisplay()
{
    while (messages_.size())
    {
        delete messages_.front();
        messages_.erase(messages_.begin());
    }

    TEST_INVARIANT;
}

void MachGuiMessageDisplay::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void MachGuiMessageDisplay::addMessage(const char* displayText, PhysRelativeTime timeToDisplay)
{
    DisplayMessage* dm = new DisplayMessage;
    dm->messageText_ = displayText;
    dm->duration_ = timeToDisplay;
    dm->startTime_ = SimManager::instance().currentTime();
    if (messages_.size() > 10)
    {
        delete messages_.front();
        messages_.erase(messages_.begin());
    }
    messages_.push_back(dm);
}

void MachGuiMessageDisplay::doOutput(std::ostream& o)
{
    if (messages_.size() == 0)
        return;
    for (int i = messages_.size() - 1; i > -1;)
    {
        if ((messages_[i]->startTime_ + messages_[i]->duration_) < SimManager::instance().currentTime())
        {
            delete messages_[i];
            messages_.erase(messages_.begin() + i);
            if (i == 0 || i == messages_.size())
                i = -1;
        }
        else
        {
            o << messages_[i]->messageText_ << std::endl;
            --i;
        }
    }
}

std::ostream& operator<<(std::ostream& o, const MachGuiMessageDisplay& t)
{

    o << "MachGuiMessageDisplay " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMessageDisplay " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

/* End DISPMESS.CPP *************************************************/
