/*
 * C H A T M S G S . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/InGameChatMessages.hpp"

#include "machgui/gui.hpp"
#include "phys/phys.hpp"
#include "ctl/List.hpp"
#include "machgui/MessageBroker.hpp"
#include "machgui/StartupScreens.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "gui/Font.hpp"
#include <fstream>

// static
MachGuiInGameChatMessages& MachGuiInGameChatMessages::instance()
{
    static MachGuiInGameChatMessages instance_;
    return instance_;
}

MachGuiInGameChatMessages::MachGuiInGameChatMessages()
    : lastUpdateTime_(Phys::time())
{
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
    MachGuiSoundManager::instance().playSound("gui/sounds/chatmsg.wav");

    // Chop up text if it is too long to fit on one line
    GuiBmpFont font = Gui::getFont(MachGui::getScaledImagePath("gui/menu/promtfnt.bmp"));
    strings choppedUpText = MachGuiMenuText::chopUpText(message, reqWidth(), font);

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
    chatMessages_.erase(chatMessages_.begin(), chatMessages_.end());
}

const ctl_list<std::string>& MachGuiInGameChatMessages::messages() const
{
    return chatMessages_;
}

void MachGuiInGameChatMessages::initialise(MachGuiMessageBroker* pMessageBroker, MachGuiStartupScreens* pStartupScreens)
{
    pMessageBroker_ = pMessageBroker;
    pStartupScreens_ = pStartupScreens;
}

void MachGuiInGameChatMessages::sendMessage(const std::string& message, MachPhys::Race intendedRace)
{
    PRE(pMessageBroker_);

    pMessageBroker_->sendInGameChatMessage(message, intendedRace);
}

const std::string& MachGuiInGameChatMessages::playerName() const
{
    return pStartupScreens_->startupData()->playerName();
}

MachPhys::Race MachGuiInGameChatMessages::playerRace() const
{
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
    GuiBmpFont font = Gui::getFont(MachGui::getScaledImagePath("gui/menu/promtfnt.bmp"));

    return (font.height() + 1 * Gui::uiScaleFactor()) * 5;
}

const std::vector<std::string>& MachGuiInGameChatMessages::standardMessages() const
{
    return standardMessages_;
}

/* End CHATMSGS.CPP *************************************************/
