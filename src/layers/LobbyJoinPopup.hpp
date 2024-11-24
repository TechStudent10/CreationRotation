#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LobbyJoinPopup : public geode::Popup<> {
protected:
    bool setup() override;
public:
    static LobbyJoinPopup* create();
};
