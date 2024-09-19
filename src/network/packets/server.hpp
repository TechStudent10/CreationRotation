#pragma once
#include "packet.hpp"

#include <types/lobby.hpp>
#include <utils.hpp>

using namespace cr::utils;

class LobbyCreatedPacket : public Packet {
    CR_PACKET(1001, LobbyCreatedPacket)

    LobbyCreatedPacket() {}
    LobbyCreatedPacket(LobbyInfo info): info(info) {} // doesn't need to impl'd

    LobbyInfo info;

    void from_msg(sio::message::ptr msg) override {
        auto map = msg->get_map()["info"]->get_map();
        info.code = map["code"]->get_string();

        info.settings = getSettingsFromMsgMap(map["settings"]->get_map());
        info.accounts = getAccountsFromMsgVector(map["accounts"]->get_vector());
    }
};

class RecieveAccountsPacket : public Packet {
    CR_PACKET(1002, RecieveAccountsPacket);

    RecieveAccountsPacket() {}
    RecieveAccountsPacket(std::vector<Account> accounts) : accounts(accounts) {}

    std::vector<Account> accounts;

    void from_msg(sio::message::ptr msg) override {
        accounts = getAccountsFromMsgVector(msg->get_map()["accounts"]->get_vector());
    }
};

class RecieveLobbyInfoPacket : public Packet {
    CR_PACKET(1003, RecieveLobbyInfoPacket);

    RecieveLobbyInfoPacket() {}
    RecieveLobbyInfoPacket(LobbyInfo info) : info(info) {}

    LobbyInfo info;

    void from_msg(sio::message::ptr msg) override {
        auto response = msg->get_map()["info"]->get_map();

        info.code = response["code"]->get_string();

        info.accounts = getAccountsFromMsgVector(response["accounts"]->get_vector());
        info.settings = getSettingsFromMsgMap(response["settings"]->get_map());
    }
};

class LobbyUpdatedPacket : public Packet {
    CR_PACKET(1004, LobbyUpdatedPacket)

    LobbyUpdatedPacket() {}
    LobbyUpdatedPacket(LobbyInfo info) : info(info) {}

    LobbyInfo info;

    void from_msg(sio::message::ptr msg) override {
        if (msg) {
            auto response = msg->get_map()["info"]->get_map();

            info.code = response["code"]->get_string();

            info.accounts = getAccountsFromMsgVector(response["accounts"]->get_vector());
            info.settings = getSettingsFromMsgMap(response["settings"]->get_map());
        }
    }
};

class JoinedLobbyPacket : public Packet {
    CR_PACKET(1007, JoinedLobbyPacket)

    JoinedLobbyPacket() {}

    void from_msg(sio::message::ptr) override {}
};

// LEVEL SWAP //

class SwapStartedPacket : public Packet {
    CR_PACKET(1005, SwapStartedPacket)

    struct AccWithIndex {
        int index;
        std::string accID;
    };

    using Accounts = std::vector<AccWithIndex>;

    SwapStartedPacket() {}
    SwapStartedPacket(Accounts accounts) : accounts(accounts) {}

    Accounts accounts;

    void from_msg(sio::message::ptr msg) override {
        auto responses = msg->get_map()["accounts"]->get_vector();
        for (auto response : responses) {
            accounts.push_back({
                .index = static_cast<int>(response->get_map()["index"]->get_int()),
                .accID = response->get_map()["accID"]->get_string()
            });
        }
    }
};

class TimeToSwapPacket : public Packet {
    CR_PACKET(1006, TimeToSwapPacket)

    TimeToSwapPacket() {}

    void from_msg(sio::message::ptr msg) override {}
};

class RecieveSwappedLevelPacket : public Packet {
    CR_PACKET(3002, RecieveSwappedLevelPacket)

    RecieveSwappedLevelPacket() {}
    RecieveSwappedLevelPacket(std::vector<std::string_view> levels): levels(levels) {}

    std::vector<std::string_view> levels;

    void from_msg(sio::message::ptr msg) override {
        for (auto strMsg : msg->get_map()["levels"]->get_vector()) {
            levels.push_back(strMsg->get_string());
        }
    }
};

class SwapEndedPacket : public Packet {
    CR_PACKET(3003, SwapEndedPacket)

    SwapEndedPacket() {}

    void from_msg(sio::message::ptr) override {}
};

// OTHER STUFF //

class ErrorPacket : public Packet {
    CR_PACKET(4001, ErrorPacket)

    ErrorPacket() {}
    ErrorPacket(std::string errorStr) : errorStr(errorStr) {}
    
    std::string errorStr;

    void from_msg(sio::message::ptr msg) override {
        if (msg) {
            errorStr = msg->get_map()["error"]->get_string();
        }
    }
};
