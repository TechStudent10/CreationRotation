#pragma once
#include "packet.hpp"

#include <types/lobby.hpp>
#include <utils.hpp>

using namespace cr::utils;

class LobbyCreatedPacket : public Packet {
    CR_PACKET(1001, LobbyCreatedPacket)

    LobbyCreatedPacket(LobbyInfo info):
        info(info) {}

    LobbyInfo info;

    CR_SERIALIZE(
        CEREAL_NVP(info)
    )
};

class RecieveAccountsPacket : public Packet {
    CR_PACKET(1002, RecieveAccountsPacket)

    RecieveAccountsPacket(std::vector<Account> accounts):
        accounts(accounts) {}

    std::vector<Account> accounts;

    CR_SERIALIZE(
        CEREAL_NVP(accounts)
    )
};

class RecieveLobbyInfoPacket : public Packet {
    CR_PACKET(1003, RecieveLobbyInfoPacket)

    RecieveLobbyInfoPacket(LobbyInfo info):
        info(info) {}

    LobbyInfo info;

    CR_SERIALIZE(
        CEREAL_NVP(info)
    )
};

class LobbyUpdatedPacket : public Packet {
    CR_PACKET(1004, LobbyUpdatedPacket)

    LobbyUpdatedPacket(LobbyInfo info):
        info(info) {}

    LobbyInfo info;

    CR_SERIALIZE(
        CEREAL_NVP(info)
    )
};

class JoinedLobbyPacket : public Packet {
    CR_PACKET(1007, JoinedLobbyPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

class RecievePublicLobbiesPacket : public Packet {
    CR_PACKET(1008, RecievePublicLobbiesPacket)

    RecievePublicLobbiesPacket(std::vector<LobbyInfo> lobbies) :
        lobbies(lobbies) {}

    std::vector<LobbyInfo> lobbies;

    CR_SERIALIZE(
        CEREAL_NVP(lobbies)
    )
};

class MessageSentPacket : public Packet {
    CR_PACKET(1009, MessageSentPacket)

    MessageSentPacket(Message message) :
        message(message) {}

    Message message;

    CR_SERIALIZE(
        CEREAL_NVP(message)
    )
};
    
// LEVEL SWAP //

struct AccWithIndex {
    int index;
    int accID;

    CR_SERIALIZE(
        CEREAL_NVP(index),
        CEREAL_NVP(accID)
    )
};

class SwapStartedPacket : public Packet {
    CR_PACKET(1005, SwapStartedPacket)

    using Accounts = std::vector<AccWithIndex>;

    SwapStartedPacket(Accounts accounts):
        accounts(accounts) {}

    Accounts accounts;

    CR_SERIALIZE(
        CEREAL_NVP(accounts)
    )
};

class TimeToSwapPacket : public Packet {
    CR_PACKET(1006, TimeToSwapPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

class RecieveSwappedLevelPacket : public Packet {
    CR_PACKET(3002, RecieveSwappedLevelPacket)

    RecieveSwappedLevelPacket(std::vector<std::string> levels):
        levels(levels) {}

    std::vector<std::string> levels;

    CR_SERIALIZE(
        CEREAL_NVP(levels)
    )
};

class SwapEndedPacket : public Packet {
    CR_PACKET(3003, SwapEndedPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

// OTHER STUFF //

class ErrorPacket : public Packet {
    CR_PACKET(4001, ErrorPacket)

    ErrorPacket(std::string error):
        error(error) {}
    
    std::string error;

    CR_SERIALIZE(
        CEREAL_NVP(error)
    )
};

class BannedUserPacket : public Packet {
    CR_PACKET(4002, BannedUserPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

class AuthorizedUserPacket : public Packet {
    CR_PACKET(4003, AuthorizedUserPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

class RecieveAuthCodePacket : public Packet {
    CR_PACKET(4004, RecieveAuthCodePacket)

    RecieveAuthCodePacket(std::string code, int botAccID):
        code(code), botAccID(botAccID) {}

    std::string code;
    int botAccID;

    CR_SERIALIZE(
        CEREAL_NVP(code),
        CEREAL_NVP(botAccID)
    )
};

class RecieveTokenPacket : public Packet {
    CR_PACKET(4005, RecieveTokenPacket)

    RecieveTokenPacket(std::string token):
        token(token) {}

    std::string token;

    CR_SERIALIZE(
        CEREAL_NVP(token)
    )
};

class InvalidTokenPacket : public Packet {
    CR_PACKET(4006, InvalidTokenPacket)

    std::string dummy;

    CR_SERIALIZE(
        dummy
    )
};

class LoginNotReceivedPacket : public Packet {
    CR_PACKET(4007, LoginNotReceivedPacket)

    std::string dummy;

    CR_SERIALIZE(
        dummy
    )
};
