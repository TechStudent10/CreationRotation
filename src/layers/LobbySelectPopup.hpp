#pragma once
#include <Geode/Geode.hpp>
#include <defs.hpp>

using namespace geode::prelude;

class CR_DLL LobbySelectPopup : public geode::Popup<> {
protected:
    Border* lobbyList;

    bool setup() override;
    void refresh(bool isFirstTime = false);
public:
    static LobbySelectPopup* create();
};
