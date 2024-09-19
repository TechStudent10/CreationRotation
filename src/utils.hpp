#pragma once
#include <Geode/Geode.hpp>
#include <types/lobby.hpp>
#include <sio_client.h>

using namespace geode::prelude;

namespace cr::utils {
    std::vector<Account> getAccountsFromMsgVector(std::vector<sio::message::ptr> msgVector);
    LobbySettings getSettingsFromMsgMap(std::map<std::string, sio::message::ptr> msgMap);

    void goToScene(CCScene* scene);
    void replaceScene(CCScene* scene);
    void popScene();
};
