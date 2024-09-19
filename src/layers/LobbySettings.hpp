#pragma once
#include <Geode/Geode.hpp>
#include <managers/SwapManager.hpp>

using namespace geode::prelude;

using Callback = std::function<void(LobbySettings)>;

class LobbySettingsPopup : public geode::Popup<LobbySettings, Callback> {
protected:
    bool setup(LobbySettings, Callback) override;
public:
    static LobbySettingsPopup* create(LobbySettings, Callback);
};
