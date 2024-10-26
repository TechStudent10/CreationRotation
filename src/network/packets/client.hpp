#pragma once
#include "packet.hpp"

#include <types/lobby.hpp>
#include <Geode/loader/Mod.hpp>

#include <utils.hpp>

// important login packet!
class LoginPacket : public Packet {
    CR_PACKET(5001, LoginPacket)

    std::string version = geode::Mod::get()->getVersion().toVString();
    Account account = cr::utils::createAccountType();

    CR_SERIALIZE(
        CEREAL_NVP(version),
        CEREAL_NVP(account)
    )
};

// response: LobbyCreatedPacket
class CreateLobbyPacket : public Packet {
    CR_PACKET(2001, CreateLobbyPacket)

    CreateLobbyPacket(LobbySettings settings):
        settings(settings) {}

    LobbySettings settings;

    CR_SERIALIZE(
        CEREAL_NVP(settings)
    )
};

// response: JoinedLobbyPacket
class JoinLobbyPacket : public Packet {
    CR_PACKET(2002, JoinLobbyPacket)

    JoinLobbyPacket(std::string code, Account account):
        code(code),
        account(account) {}
    
    std::string code;
    Account account;

    CR_SERIALIZE(
        CEREAL_NVP(code),
        CEREAL_NVP(account)
    )
};

// response: RecieveAccountsPacket
class GetAccountsPacket : public Packet {
    CR_PACKET(2003, GetAccountsPacket)

    GetAccountsPacket(std::string code):
        code(code) {}

    std::string code;

    CR_SERIALIZE(
        CEREAL_NVP(code)
    )
};

// response: RecieveLobbyInfoPacket
class GetLobbyInfoPacket : public Packet {
    CR_PACKET(2004, GetLobbyInfoPacket)

    GetLobbyInfoPacket(std::string code):
        code(code) {}

    std::string code;

    CR_SERIALIZE(
        CEREAL_NVP(code)
    )
};

class DisconnectFromLobbyPacket : public Packet {
    CR_PACKET(2005, DisconnectFromLobbyPacket)

    std::string dummy;

    CR_SERIALIZE(dummy)
};

class UpdateLobbyPacket : public Packet {
    CR_PACKET(2006, UpdateLobbyPacket)

    UpdateLobbyPacket(std::string code, LobbySettings settings):
        code(code),
        settings(settings) {}

    std::string code;
    LobbySettings settings;

    CR_SERIALIZE(
        CEREAL_NVP(code),
        CEREAL_NVP(settings)
    )
};

class KickUserPacket : public Packet {
    CR_PACKET(2008, KickUserPacket)

    KickUserPacket(int userID) :
        userID(userID) {}

    int userID;

    CR_SERIALIZE(
        CEREAL_NVP(userID)
    )
};

// response: RecievePublicLobbiesPacket
class GetPublicLobbiesPacket : public Packet {
    CR_PACKET(2009, GetPublicLobbiesPacket)

    std::string dummy;

    CR_SERIALIZE(dummy);
};

// SWAPPING

class StartSwapPacket : public Packet {
    CR_PACKET(2007, StartSwapPacket)

    StartSwapPacket(std::string code):
        code(code) {}

    std::string code;

    CR_SERIALIZE(
        CEREAL_NVP(code)
    )
};

class SendLevelPacket : public Packet {
    CR_PACKET(3001, SendLevelPacket)

    SendLevelPacket(std::string code, int accIdx, std::string lvlStr):
        code(code),
        accIdx(accIdx),
        lvlStr(lvlStr) {}

    int accIdx;
    std::string lvlStr;
    std::string code;

    CR_SERIALIZE(
        CEREAL_NVP(accIdx),
        CEREAL_NVP(lvlStr),
        CEREAL_NVP(code)
    )
};
