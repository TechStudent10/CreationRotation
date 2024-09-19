#pragma once
#include "packet.hpp"

#include <types/lobby.hpp>

#define FROM_MSG() void from_msg(sio::message::ptr msg) override {}

// response: LobbyCreatedPacket
class CreateLobbyPacket : public Packet {
    CR_PACKET(2001, CreateLobbyPacket)

    CreateLobbyPacket() {}
    CreateLobbyPacket(LobbySettings settings) {
        members["name"] = sio::string_message::create(settings.name);
        members["turns"] = sio::int_message::create(settings.turns);
        members["owner"] = sio::int_message::create(settings.owner);
        members["minutesPerTurn"] = sio::int_message::create(settings.minutesPerTurn);
    }

    FROM_MSG()
};

class JoinLobbyPacket : public Packet {
    CR_PACKET(2002, JoinLobbyPacket)

    JoinLobbyPacket() {}
    JoinLobbyPacket(std::string code, Account account) {
        members["code"] = sio::string_message::create(code);

        std::map<std::string, sio::message::ptr> accountMap;
        
        #define STRMSG(str) sio::string_message::create(str)

        accountMap["name"] = STRMSG(account.name);
        accountMap["userID"] = STRMSG(account.userID);
        accountMap["iconID"] = STRMSG(account.iconID);
        accountMap["color1"] = STRMSG(account.color1);
        accountMap["color2"] = STRMSG(account.color2);
        accountMap["color3"] = STRMSG(account.color3);

        members["account"] = sio::object_message::create();
        members["account"]->get_map() = accountMap;
    }

    FROM_MSG()
};

// response: RecieveAccountsPacket
class GetAccountsPacket : public Packet {
    CR_PACKET(2003, GetAccountsPacket)

    GetAccountsPacket() {}
    GetAccountsPacket(std::string code) {
        members["code"] = sio::string_message::create(code);
    }

    FROM_MSG()
};

// response: RecieveLobbyInfoPacket
class GetLobbyInfoPacket : public Packet {
    CR_PACKET(2004, GetLobbyInfoPacket)

    GetLobbyInfoPacket() {}
    GetLobbyInfoPacket(std::string code) {
        members["code"] = sio::string_message::create(code);
    }

    FROM_MSG()
};

class DisconnectFromLobbyPacket : public Packet {
    CR_PACKET(2005, DisconnectFromLobbyPacket)

    DisconnectFromLobbyPacket() {}

    FROM_MSG()
};

class UpdateLobbyPacket : public Packet {
    CR_PACKET(2006, UpdateLobbyPacket);

    UpdateLobbyPacket() {}
    UpdateLobbyPacket(std::string code, LobbySettings settings) { 
        members["code"] = sio::string_message::create(code);
        members["name"] = sio::string_message::create(settings.name);
        members["turns"] = sio::int_message::create(
            settings.turns
        );
    }

    FROM_MSG()
};

// SWAPPING

class StartSwapPacket : public Packet {
    CR_PACKET(2007, StartSwapPacket)

    StartSwapPacket() {}
    StartSwapPacket(std::string code) {
        members["code"] = sio::string_message::create(code);
    }

    FROM_MSG()
};

class SendLevelPacket : public Packet {
    CR_PACKET(3001, SendLevelPacket)

    SendLevelPacket() {}
    SendLevelPacket(std::string code, int accIdx, std::string lvlStr) : code(code), accIdx(accIdx), lvlStr(lvlStr) {
        members["code"] = sio::string_message::create(code);
        members["accIdx"] = sio::int_message::create(accIdx);
        members["lvlStr"] = sio::string_message::create(lvlStr);
    }

    int accIdx;
    std::string_view lvlStr;
    std::string_view code;

    FROM_MSG()
};
