/*
 * M E S S B R O K . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/MessageBroker.hpp"

#include <stdlib.h>
#include "base/Diag.hpp"
#include "gui/ResourceString.hpp"
#include "network/Message.hpp"
#include "machgui/MessageDisplay.hpp"
#include "machgui/ChatWindow.hpp"
#include "machgui/StartupData.hpp"
#include "machgui/InGameChatMessages.hpp"
#include "machgui/internal/strings.hpp"
#include "machgui/internal/SoundManager.hpp"
#include "system/PathName.hpp"

#pragma pack(push, 1)
struct MachGuiChatMessage
{
    MachLogMessageHeader header_;
    char chat_[MAX_CHATMESSAGE_LEN + 1];
};

struct MachGuiJoinMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
    int uniqueMachineNumber_;
};

struct MachGuiUpdatePlayersMessage
{
    MachLogMessageHeader header_;
    MachGuiStartupData::PlayerInfo playerInfo_[4];
};

struct MachGuiUpdateGameSettingsMessage
{
    MachLogMessageHeader header_;
    MachGuiStartupData::GameSettings gameSettings_;
};

struct MachGuiRaceChangeRequestMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
    MachPhys::Race race_;
    int index_;
};

struct MachGuiImReadyMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
    bool ready_;
};

struct MachGuiStartMessage
{
    MachLogMessageHeader header_;
};

struct MachGuiHostCancelMessage
{
    MachLogMessageHeader header_;
};

struct MachGuiClientCancelMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
};

struct MachGuiInGameChatMessage
{
    MachLogMessageHeader header_;
    char chatMessage_[MAX_INGAMECHATMESSAGE_LEN + 1];
    MachPhys::Race race_; // Who the message is intended for
};

struct MachGuiHasMachinesCDMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
    bool hasMachinesCD_;
};

struct MachGuiIveLostMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
};

struct MachGuiNameChangeMessage
{
    MachLogMessageHeader header_;
    char playerName_[MAX_PLAYERNAME_LEN + 1];
    int uniqueMachineNumber_;
};
#pragma pack(pop)

MachGuiMessageBroker::MachGuiMessageBroker(MachGuiStartupData* pStartupData)
    : pStartupData_(pStartupData)
{

    TEST_INVARIANT;
}

// virtual
MachGuiMessageBroker::~MachGuiMessageBroker()
{
    TEST_INVARIANT;
}

void MachGuiMessageBroker::processChatMessage(MachGuiChatMessage* pMessage)
{
    // Play sound
    MachGuiSoundManager::instance().playSound("gui/sounds/chatmsg.wav");

    MachGuiChatWindow::addText(pMessage->chat_);
}

void MachGuiMessageBroker::processJoinMessage(MachGuiJoinMessage* pMessage)
{
    PRE(pStartupData_);

    pStartupData_->receivedJoinMessage(pMessage->playerName_, pMessage->uniqueMachineNumber_);
}

void MachGuiMessageBroker::processUpdatePlayersMessage(MachGuiUpdatePlayersMessage* pMessage)
{
    NETWORK_STREAM("MachGuiMessageBroker::processUpdatePlayersMessage\n");
    NETWORK_INDENT(2);
    PRE(pStartupData_);

    // Copy new information into players info structure
    memcpy(pStartupData_->players(), pMessage->playerInfo_, sizeof(MachGuiStartupData::PlayerInfo) * 4);

    // Refresh any gui components...
    pStartupData_->receivedUpdatePlayersMessage();
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiMessageBroker::processUpdatePlayersMessage DONE\n");
}

void MachGuiMessageBroker::processUpdateGameSettingsMessage(MachGuiUpdateGameSettingsMessage* pMessage)
{
    PRE(pStartupData_);

    // Copy new information into game settings structure
    memcpy(pStartupData_->gameSettings(), &pMessage->gameSettings_, sizeof(MachGuiStartupData::GameSettings));

    // Refresh any gui components...
    pStartupData_->receivedUpdateGameSettingsMessage();
}

void MachGuiMessageBroker::processRaceChangeRequest(MachGuiRaceChangeRequestMessage* pMessage)
{
    pStartupData_->receivedRaceChangeRequest(pMessage->playerName_, pMessage->index_, pMessage->race_);
}

void MachGuiMessageBroker::processImReadyMessage(MachGuiImReadyMessage* pMessage)
{
    PRE(pStartupData_);

    pStartupData_->receivedImReadyMessage(pMessage->playerName_, pMessage->ready_);
}

void MachGuiMessageBroker::processStartMessage(MachGuiStartMessage* /*pMessage*/)
{
    pStartupData_->receivedStartMessage();
}

void MachGuiMessageBroker::processHostCancelMessage(MachGuiHostCancelMessage* /*pMessage*/)
{
    pStartupData_->receivedHostCancelMessage();
}

void MachGuiMessageBroker::processClientCancelMessage(MachGuiClientCancelMessage* pMessage)
{
    pStartupData_->receivedClientCancelMessage(pMessage->playerName_);
}

void MachGuiMessageBroker::processInGameChatMessage(MachGuiInGameChatMessage* pMessage)
{
    pStartupData_->receivedInGameChatMessage(pMessage->chatMessage_, pMessage->race_);
}

void MachGuiMessageBroker::processIveLostMessage(MachGuiIveLostMessage* pMessage)
{
    GuiResourceString lostString(IDS_PLAYER_HAS_LOST, pMessage->playerName_);
    MachGuiInGameChatMessages::instance().addMessage(lostString.asString());
}

void MachGuiMessageBroker::processMachGuiMessage(NetMessage* pMessage)
{
    PRE(getSystemType(pMessage) == SYSTEM_MACHGUI_MESSAGE);

    DEBUG_STREAM(DIAG_NETWORK, "processMachGuiMessage enter messageType " << getMessageType(pMessage) << std::endl);

    switch (getMessageType(pMessage))
    {
        case MT_CHATMESSAGE:
            // processChatMessage( reinterpret_cast< MachGuiChatMessage*>(pMessage->body().body() ) );
            processChatMessage(reinterpret_cast<MachGuiChatMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_JOINMESSAGE:
            // processJoinMessage( reinterpret_cast< MachGuiJoinMessage*>(pMessage->body().body() ) );
            processJoinMessage(reinterpret_cast<MachGuiJoinMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_UPDATEPLAYERSMESSAGE:
            // processUpdatePlayersMessage( reinterpret_cast< MachGuiUpdatePlayersMessage*>(pMessage->body().body() )
            // );
            processUpdatePlayersMessage(
                reinterpret_cast<MachGuiUpdatePlayersMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_RACECHANGEREQUESTMESSAGE:
            // processRaceChangeRequest( reinterpret_cast< MachGuiRaceChangeRequestMessage*>(pMessage->body().body() )
            // );
            processRaceChangeRequest(
                reinterpret_cast<MachGuiRaceChangeRequestMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_STARTMESSAGE:
            // processStartMessage( reinterpret_cast< MachGuiStartMessage*>(pMessage->body().body() ) );
            processStartMessage(reinterpret_cast<MachGuiStartMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_IMREADYMESSAGE:
            // processImReadyMessage( reinterpret_cast< MachGuiImReadyMessage*>(pMessage->body().body() ) );
            processImReadyMessage(
                reinterpret_cast<MachGuiImReadyMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_HOSTCANCELMESSAGE:
            // processHostCancelMessage( reinterpret_cast< MachGuiHostCancelMessage*>(pMessage->body().body() ) );
            processHostCancelMessage(
                reinterpret_cast<MachGuiHostCancelMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_CLIENTCANCELMESSAGE:
            // processClientCancelMessage( reinterpret_cast< MachGuiClientCancelMessage*>(pMessage->body().body() )  );
            processClientCancelMessage(
                reinterpret_cast<MachGuiClientCancelMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MY_UPDATEGAMESETTINGSMESSAGE:
            // processUpdateGameSettingsMessage( _REINTERPRET_CAST( MachGuiUpdateGameSettingsMessage*,
            // pMessage->body().body() )  );
            processUpdateGameSettingsMessage(
                reinterpret_cast<MachGuiUpdateGameSettingsMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_INGAMECHATMESSAGE:
            // processInGameChatMessage( reinterpret_cast< MachGuiInGameChatMessage*>(pMessage->body().body() )  );
            processInGameChatMessage(
                reinterpret_cast<MachGuiInGameChatMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_HASMACHINESCDMESSAGE:
            // processHasMachinesCDMessage( reinterpret_cast< MachGuiHasMachinesCDMessage*>(pMessage->body().body() )
            // );
            processHasMachinesCDMessage(
                reinterpret_cast<MachGuiHasMachinesCDMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_IVELOSTMESSAGE:
            // processIveLostMessage( reinterpret_cast< MachGuiIveLostMessage*>(pMessage->body().body() )  );
            processIveLostMessage(
                reinterpret_cast<MachGuiIveLostMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        case MT_NAMECHANGEMESSAGE:
            // processNameChangeMessage( reinterpret_cast< MachGuiNameChangeMessage*>(pMessage->body().body() ) );
            processNameChangeMessage(
                reinterpret_cast<MachGuiNameChangeMessage*>(const_cast<uint8*>(pMessage->body().body())));
            break;
        default:
            ASSERT_BAD_CASE;
    }

    DEBUG_STREAM(DIAG_NETWORK, "processMachGuiMessage done" << std::endl);
}

// virtual
void MachGuiMessageBroker::processMessage(NetMessage* pMessage)
{
    switch (getSystemType(pMessage))
    {
        case SYSTEM_MACHLOG_MESSAGE:
            MachLogMessageBroker::processMessage(pMessage);
            break;
        case SYSTEM_MACHGUI_MESSAGE:
            processMachGuiMessage(pMessage);
            delete pMessage;
            break;
        default:
            NETWORK_STREAM("Invalid message code detected in MGMessageBroker::processMessage\n");
            NETWORK_STREAM(" message is :\n" << *pMessage << std::endl);
            ASSERT_BAD_CASE;
    }
}

MachGuiMessageBroker::SystemType MachGuiMessageBroker::getSystemType(NetMessage* pMessage)
{
    return (SystemType)pMessage->body().body()[0];
}

MachGuiMessageBroker::MessageType MachGuiMessageBroker::getMessageType(NetMessage* pMessage)
{
    //  return (MessageType)pMessage->body().body()[1];
    return (MessageType)((MachLogMessageHeader*)pMessage->body().body())->messageCode_;
}

void MachGuiMessageBroker::sendChatMessage(const std::string& chat)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiChatMessage* pMessage = reinterpret_cast<MachGuiChatMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_CHATMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiChatMessage);

    ASSERT(strlen(chat.c_str()) <= MAX_CHATMESSAGE_LEN, "chat message too long");

    strcpy(pMessage->chat_, chat.c_str());

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendJoinMessage(const std::string& playerName, int uniqueMachineNumber)
{
    NETWORK_STREAM("MachGuiMessageBroker::sendJoinMessage\n");
    NETWORK_INDENT(2);
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiJoinMessage* pMessage = reinterpret_cast<MachGuiJoinMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_JOINMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiJoinMessage);

    ASSERT(strlen(playerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, playerName.c_str());

    pMessage->uniqueMachineNumber_ = uniqueMachineNumber;

    doSend(pLogMessage);
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiMessageBroker::sendJoinMessage DONE\n");
}

void MachGuiMessageBroker::sendUpdatePlayersMessage()
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiUpdatePlayersMessage* pMessage = reinterpret_cast<MachGuiUpdatePlayersMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_UPDATEPLAYERSMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiUpdatePlayersMessage);

    memcpy(pMessage->playerInfo_, pStartupData_->players(), sizeof(MachGuiStartupData::PlayerInfo) * 4);

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendUpdateGameSettingsMessage()
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiUpdateGameSettingsMessage* pMessage = reinterpret_cast<MachGuiUpdateGameSettingsMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MY_UPDATEGAMESETTINGSMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiUpdateGameSettingsMessage);

    memcpy(&pMessage->gameSettings_, pStartupData_->gameSettings(), sizeof(MachGuiStartupData::GameSettings));

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendRaceChangeRequest(const std::string& playerName, size_t playerIndex, MachPhys::Race newRace)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiRaceChangeRequestMessage* pMessage = reinterpret_cast<MachGuiRaceChangeRequestMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_RACECHANGEREQUESTMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiRaceChangeRequestMessage);

    ASSERT(strlen(playerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, playerName.c_str());

    pMessage->race_ = newRace;
    pMessage->index_ = playerIndex;

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendImReadyMessage(const std::string& playerName, bool ready)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiImReadyMessage* pMessage = reinterpret_cast<MachGuiImReadyMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_IMREADYMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiImReadyMessage);

    ASSERT(strlen(playerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, playerName.c_str());

    pMessage->ready_ = ready;

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendStartMessage()
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiStartMessage* pMessage = reinterpret_cast<MachGuiStartMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_STARTMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiStartMessage);

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendHostCancelMessage()
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiHostCancelMessage* pMessage = reinterpret_cast<MachGuiHostCancelMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_HOSTCANCELMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiHostCancelMessage);

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendClientCancelMessage(const std::string& playerName)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiClientCancelMessage* pMessage = reinterpret_cast<MachGuiClientCancelMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_CLIENTCANCELMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiClientCancelMessage);

    ASSERT(strlen(playerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, playerName.c_str());

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendInGameChatMessage(const std::string& message, MachPhys::Race intendedForRace)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiInGameChatMessage* pMessage = reinterpret_cast<MachGuiInGameChatMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_INGAMECHATMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiInGameChatMessage);
    pMessage->race_ = intendedForRace;

    ASSERT(strlen(message.c_str()) <= MAX_INGAMECHATMESSAGE_LEN, "chat message too long");

    strcpy(pMessage->chatMessage_, message.c_str());

    doSend(pLogMessage);
}

void MachGuiMessageBroker::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMessageBroker& t)
{

    o << "MachGuiMessageBroker " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMessageBroker " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

std::ostream& operator<<(std::ostream& o, const MachGuiMessageBroker::MessageType& t)
{
    switch (t)
    {
        case MachGuiMessageBroker::MT_CHATMESSAGE:
            o << "MT_CHATMESSAGE";
            break;
        case MachGuiMessageBroker::MT_JOINMESSAGE:
            o << "MT_JOINMESSAGE";
            break;
        case MachGuiMessageBroker::MT_UPDATEPLAYERSMESSAGE:
            o << "MT_UPDATEPLAYERSMESSAGE";
            break;
        case MachGuiMessageBroker::MT_RACECHANGEREQUESTMESSAGE:
            o << "MT_RACECHANGEREQUESTMESSAGE";
            break;
        case MachGuiMessageBroker::MT_IMREADYMESSAGE:
            o << "MT_IMREADYMESSAGE";
            break;
        case MachGuiMessageBroker::MT_STARTMESSAGE:
            o << "MT_STARTMESSAGE";
            break;
        case MachGuiMessageBroker::MT_HOSTCANCELMESSAGE:
            o << "MT_HOSTCANCELMESSAGE";
            break;
        case MachGuiMessageBroker::MT_CLIENTCANCELMESSAGE:
            o << "MT_CLIENTCANCELMESSAGE";
            break;
        case MachGuiMessageBroker::MY_UPDATEGAMESETTINGSMESSAGE:
            o << "MY_UPDATEGAMESETTINGSMESSAGE";
            break;
        case MachGuiMessageBroker::MT_INGAMECHATMESSAGE:
            o << "MT_INGAMECHATMESSAGE";
            break;
        case MachGuiMessageBroker::MT_HASMACHINESCDMESSAGE:
            o << "MT_HASMACHINESCDMESSAGE";
            break;
        case MachGuiMessageBroker::MT_IVELOSTMESSAGE:
            o << "MT_IVELOSTMESSAGE";
            break;
        default:
            o << "Unknown message code " << (int)t << std::endl;
    }
    return o;
}

void MachGuiMessageBroker::sendHasMachinesCDMessage(const std::string& playerName, bool hasMachinesCD)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiHasMachinesCDMessage* pMessage = reinterpret_cast<MachGuiHasMachinesCDMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_HASMACHINESCDMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiHasMachinesCDMessage);

    ASSERT(strlen(playerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, playerName.c_str());

    pMessage->hasMachinesCD_ = hasMachinesCD;

    doSend(pLogMessage);
}

void MachGuiMessageBroker::processHasMachinesCDMessage(MachGuiHasMachinesCDMessage* pMessage)
{
    PRE(pStartupData_);

    pStartupData_->receivedHasMachinesCDMessage(pMessage->playerName_, pMessage->hasMachinesCD_);
}

void MachGuiMessageBroker::sendIveLostMessage(const std::string& playerName)
{
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiIveLostMessage* pMessage = reinterpret_cast<MachGuiIveLostMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_IVELOSTMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiIveLostMessage);

    strcpy(pMessage->playerName_, playerName.c_str());

    doSend(pLogMessage);
}

void MachGuiMessageBroker::sendNameChangeMessage(const std::string& newPlayerName, int uniqueMachineNumber)
{
    NETWORK_STREAM("MachGuiMessageBroker::sendJoinMessage\n");
    NETWORK_INDENT(2);
    MachLogNetMessage* pLogMessage = new MachLogNetMessage();
    MachGuiNameChangeMessage* pMessage = reinterpret_cast<MachGuiNameChangeMessage*>(pLogMessage);
    pMessage->header_.systemCode_ = SYSTEM_MACHGUI_MESSAGE;
    pMessage->header_.messageCode_ = MT_NAMECHANGEMESSAGE;
    pMessage->header_.totalLength_ = sizeof(MachGuiNameChangeMessage);

    ASSERT(strlen(newPlayerName.c_str()) <= MAX_PLAYERNAME_LEN, "player name too long");

    strcpy(pMessage->playerName_, newPlayerName.c_str());

    pMessage->uniqueMachineNumber_ = uniqueMachineNumber;

    doSend(pLogMessage);
    NETWORK_INDENT(-2);
    NETWORK_STREAM("MachGuiMessageBroker::sendJoinMessage DONE\n");
}

void MachGuiMessageBroker::processNameChangeMessage(MachGuiNameChangeMessage* pMessage)
{
    pStartupData_->receivedNameChangeMessage(pMessage->playerName_, pMessage->uniqueMachineNumber_);
}

/* End MESSBROK.CPP *************************************************/
