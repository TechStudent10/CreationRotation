#pragma once
#include <string>
#include <vector>
#include <matjson.hpp>

// #define JSON_PROP(__name) {#__name, matjson::Value(__name)}
// #define JSON_PROP_WITH_NAME(__name, __value) {#__name, __value}

// #define JSON_STRUCT(...) std::string toJson() { \
//     return matjson::Value({__VA_ARGS__}).dump(); \
//     }

struct LobbySettings {
    std::string name;
    int turns;
    int owner;
    int minutesPerTurn;
};

struct Account {
    std::string name;
    std::string userID;
    std::string iconID;
    std::string color1;
    std::string color2;
    std::string color3;
};

struct LobbyInfo {
    std::string code;
    LobbySettings settings;
    std::vector<Account> accounts;
};
