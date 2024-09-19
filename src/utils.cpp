#include "utils.hpp"

std::vector<Account> cr::utils::getAccountsFromMsgVector(std::vector<sio::message::ptr> vec) {
    std::vector<Account> accounts;
    for (auto _msg : vec) {
        auto msg = _msg->get_map();
        #define GETKEY(key) msg.at(key)->get_string()
        Account acc = {
            .name = GETKEY("name"),
            .userID = GETKEY("userID"),
            .iconID = GETKEY("iconID"),
            .color1 = GETKEY("color1"),
            .color2 = GETKEY("color2"),
            .color3 = GETKEY("color3")
        };
        accounts.push_back(acc);
    }
    return accounts;
}

LobbySettings cr::utils::getSettingsFromMsgMap(std::map<std::string, sio::message::ptr> settingsMap) {
    LobbySettings settings;

    settings.name = settingsMap["name"]->get_string();
    settings.turns = settingsMap["turns"]->get_int();
    settings.owner = settingsMap["owner"]->get_int();

    return settings;
}

void cr::utils::goToScene(CCScene *scene) {
    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
}

void cr::utils::replaceScene(CCScene *scene) {
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
}

void cr::utils::popScene() {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}
