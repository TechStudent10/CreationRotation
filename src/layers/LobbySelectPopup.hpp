#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LobbySelectPopup : public geode::Popup<> {
protected:
    Border* lobbyList;

    bool setup() override;
    void refresh(bool isFirstTime = false);
public:
    static LobbySelectPopup* create();
};
