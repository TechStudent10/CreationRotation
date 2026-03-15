#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LobbyJoinPopup : public geode::Popup {
protected:
    bool init() override;
public:
    static LobbyJoinPopup* create();
};
