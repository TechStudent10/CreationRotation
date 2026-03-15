#pragma once
#include <Geode/Geode.hpp>
#include <managers/SwapManager.hpp>

using namespace geode::prelude;

using Callback = geode::Function<void(LobbySettings)>;

class LobbySettingsPopup : public geode::Popup {
protected:
    bool init(LobbySettings const&, Callback);
public:
    static LobbySettingsPopup* create(LobbySettings const&, Callback);
};
