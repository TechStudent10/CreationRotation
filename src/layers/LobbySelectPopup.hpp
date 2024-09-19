#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class LobbySelectPopup : public geode::Popup<> {
protected:
    bool setup() override;
public:
    static LobbySelectPopup* create();
};
