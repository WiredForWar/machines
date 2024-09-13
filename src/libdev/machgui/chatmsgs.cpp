/*
 * C H A T M S G S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/chatmsgs.hpp"

#include "machgui/gui.hpp"
#include "phys/phys.hpp"
#include "stdlib/string.hpp"
#include "ctl/list.hpp"
#include "machgui/messbrok.hpp"
#include "machgui/startup.hpp"
#include "machgui/strtdata.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/internal/mgsndman.hpp"
#include "gui/font.hpp"
#include <fstream>

struct MachGuiInGameChatMessagesImpl
{
    MachGuiInGameChatMessagesImpl()
        : messageAdded_(false)
        , lastUpdateTime_(Phys::time())
        , pMessageBroker_(nullptr)
        , pStartupScreens_(nullptr)
    {
    }

    ctl_list<std::string> chatMessages_;
    PhysAbsoluteTime lastUpdateTime_;
    bool messageAdded_;
    MachGuiMessageBroker* pMessageBroker_;
    MachGuiStartupScreens* pStartupScreens_;
    std::vector<std::string> standardMessages_;
};

#define CB_MachGuiInGameChatMessagesImpl_DEPIMPL()                                                                     \
    PRE(pImpl_)                                                                                                        \
    CB_DEPIMPL(ctl_list<std::string>, chatMessages_)                                                                   \
    CB_DEPIMPL(PhysAbsoluteTime, lastUpdateTime_)                                                                      \
    CB_DEPIMPL(bool, messageAdded_)                                                                                    \
    CB_DEPIMPL(MachGuiMessageBroker*, pMessageBroker_)                                                                 \
    CB_DEPIMPL(MachGuiStartupScreens*, pStartupScreens_)                                                               \
    CB_DEPIMPL(std::vector<std::string>, standardMessages_)

// static
MachGuiInGameChatMessages& MachGuiInGameChatMessages::instance()
{
    static MachGuiInGameChatMessages instance_;
    return instance_;
}

MachGuiInGameChatMessages::MachGuiInGameChatMessages()
    : pImpl_(new MachGuiInGameChatMessagesImpl())
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();
    standardMessages_.reserve(32);

    // std::ifstream chatMessagesFile( "data/chatmsgs.txt", std::ios::in | std::ios::nocreate );
    std::ifstream chatMessagesFile("data/chatmsgs.txt", std::ios::in);
    char buffer[MAX_INGAMECHATMESSAGE_LEN - MAX_PLAYERNAME_LEN - 5];

    if (chatMessagesFile.is_open())
    {
        while (! chatMessagesFile.eof())
        {
            chatMessagesFile.getline(buffer, sizeof(buffer));
            standardMessages_.push_back(buffer);
        }
    }

    TEST_INVARIANT;
}

MachGuiInGameChatMessages::~MachGuiInGameChatMessages()
{
    TEST_INVARIANT;

    delete pImpl_;
}

void MachGuiInGameChatMessages::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiInGameChatMessages& t)
{

    o << "MachGuiInGameChatMessages " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiInGameChatMessages " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

bool MachGuiInGameChatMessages::update()
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    bool returnVal = messageAdded_;

    messageAdded_ = false;

    // Clear out oldest message every five seconds
    if (Phys::time() - lastUpdateTime_ > 8.0)
    {
        if (!chatMessages_.empty())
        {
            chatMessages_.pop_front();
            returnVal = true;
        }

        lastUpdateTime_ = Phys::time();
    }

    return returnVal;
}

void MachGuiInGameChatMessages::addMessage(const std::string& message)
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    MachGuiSoundManager::instance().playSound("gui/sounds/chatmsg.wav");

    // Chop up text if it is too long to fit on one line
    strings choppedUpText;
    GuiBmpFont font = GuiBmpFont::getFont(MachGui::getScaledImagePath("gui/menu/promtfnt.bmp"));
    MachGuiMenuText::chopUpText(message, reqWidth(), font, &choppedUpText);

    // Add text to list of chat messages
    for (strings::iterator iter = choppedUpText.begin(); iter != choppedUpText.end(); ++iter)
    {
        chatMessages_.push_back(*iter);
    }

    // Only five chat messages are viewable at any one time
    while (chatMessages_.size() > 5)
        chatMessages_.pop_front();

    lastUpdateTime_ = Phys::time();
    messageAdded_ = true;
}

void MachGuiInGameChatMessages::clearAllMessages()
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    chatMessages_.erase(chatMessages_.begin(), chatMessages_.end());
}

const ctl_list<std::string>& MachGuiInGameChatMessages::messages() const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    return chatMessages_;
}

void MachGuiInGameChatMessages::initialise(MachGuiMessageBroker* pMessageBroker, MachGuiStartupScreens* pStartupScreens)
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    pMessageBroker_ = pMessageBroker;
    pStartupScreens_ = pStartupScreens;
}

void MachGuiInGameChatMessages::sendMessage(const std::string& message, MachPhys::Race intendedRace)
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    PRE(pMessageBroker_);

    pMessageBroker_->sendInGameChatMessage(message, intendedRace);
}

const std::string& MachGuiInGameChatMessages::playerName() const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    return pStartupScreens_->startupData()->playerName();
}

MachPhys::Race MachGuiInGameChatMessages::playerRace() const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    MachPhys::Race playersRace = MachPhys::N_RACES;

    for (int i = 0; i < 4 && playersRace == MachPhys::N_RACES; ++i)
    {
        if (pStartupScreens_->startupData()->players()[i].getDisplayName() == playerName())
        {
            playersRace = pStartupScreens_->startupData()->players()[i].race_;
        }
    }

    return playersRace;
}

bool MachGuiInGameChatMessages::opponentExists(int index) const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    bool returnVal = false;
    int opponentIndex = 0;

    for (int i = 0; i < 4 && ! returnVal; ++i)
    {
        if (pStartupScreens_->startupData()->players()[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN
            && pStartupScreens_->startupData()->players()[i].getDisplayName() != playerName())
        {
            if (index == opponentIndex)
            {
                returnVal = true;
            }
            ++opponentIndex;
        }
    }

    return returnVal;
}

std::string MachGuiInGameChatMessages::opponentName(int index) const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    PRE(opponentExists(index));

    std::string returnVal;
    bool opponentFound = false;
    int opponentIndex = 0;

    for (int i = 0; i < 4 && ! opponentFound; ++i)
    {
        if (pStartupScreens_->startupData()->players()[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN
            && pStartupScreens_->startupData()->players()[i].getDisplayName() != playerName())
        {
            if (index == opponentIndex)
            {
                opponentFound = true;
                returnVal = pStartupScreens_->startupData()->players()[i].getDisplayName();
            }
            ++opponentIndex;
        }
    }

    return returnVal;
}

MachPhys::Race MachGuiInGameChatMessages::opponentRace(int index) const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    PRE(opponentExists(index));

    MachPhys::Race returnVal = MachPhys::N_RACES;
    bool opponentFound = false;
    int opponentIndex = 0;

    for (int i = 0; i < 4 && ! opponentFound; ++i)
    {
        if (pStartupScreens_->startupData()->players()[i].status_ == MachGuiStartupData::PlayerInfo::HUMAN
            && pStartupScreens_->startupData()->players()[i].getDisplayName() != playerName())
        {
            if (index == opponentIndex)
            {
                opponentFound = true;
                returnVal = pStartupScreens_->startupData()->players()[i].race_;
            }
            ++opponentIndex;
        }
    }

    return returnVal;
}

// static
int MachGuiInGameChatMessages::reqWidth()
{
    return 430 * Gui::uiScaleFactor();
}

// static
int MachGuiInGameChatMessages::reqHeight()
{
    GuiBmpFont font = GuiBmpFont::getFont(MachGui::getScaledImagePath("gui/menu/promtfnt.bmp"));

    return (font.charHeight() + 1 * Gui::uiScaleFactor()) * 5;
}

const std::vector<std::string>& MachGuiInGameChatMessages::standardMessages() const
{
    CB_MachGuiInGameChatMessagesImpl_DEPIMPL();

    return standardMessages_;
}

/* End CHATMSGS.CPP *************************************************/
