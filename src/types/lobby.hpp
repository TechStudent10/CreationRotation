#pragma once
#include <string>
#include <vector>

#include <serialization.hpp>

struct LobbySettings {
    std::string name;
    // std::string password;
    int turns;
    int owner;
    int minutesPerTurn;

    CR_SERIALIZE(
        CEREAL_NVP(name),
        // CEREAL_NVP(password),
        CEREAL_NVP(turns),
        CEREAL_NVP(owner),
        CEREAL_NVP(minutesPerTurn)
    )
};

struct Account {
    std::string name;
    int userID;
    int iconID;
    int color1;
    int color2;
    int color3;

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
