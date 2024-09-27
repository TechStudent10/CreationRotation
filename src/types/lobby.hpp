#pragma once
#include <string>
#include <vector>

#include <serialization.hpp>

struct LobbySettings {
    std::string name;
    int turns;
    int owner;
    int minutesPerTurn;

    CR_SERIALIZE(
        CEREAL_NVP(name),
        CEREAL_NVP(turns),
        CEREAL_NVP(owner),
        CEREAL_NVP(minutesPerTurn)
    )
};

struct Account {
    std::string name;
    std::string userID;
    std::string iconID;
    std::string color1;
    std::string color2;
    std::string color3;

    CR_SERIALIZE(
        CEREAL_NVP(name),
        CEREAL_NVP(userID),
        CEREAL_NVP(iconID),
        CEREAL_NVP(color1),
        CEREAL_NVP(color2),
        CEREAL_NVP(color3)
    )
};

struct LobbyInfo {
    std::string code;
    LobbySettings settings;
    std::vector<Account> accounts;

    CR_SERIALIZE(
        CEREAL_NVP(code),
        CEREAL_NVP(settings),
        CEREAL_NVP(accounts)
    )
};
