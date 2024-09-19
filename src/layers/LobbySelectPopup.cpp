#include "LobbySelectPopup.hpp"

#include <managers/SwapManager.hpp>

#include <layers/Lobby.hpp>
#include <layers/LobbySettings.hpp>
#include <layers/LobbyJoinPopup.hpp>

#include <utils.hpp>

LobbySelectPopup* LobbySelectPopup::create() {
    auto ret = new LobbySelectPopup;
    if (ret->initAnchored(280.f, 180.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LobbySelectPopup::setup() {
    this->setTitle("Creation Rotation");

    auto mainMenu = CCMenu::create();

    auto joinBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create(
            "Join Lobby",
            "bigFont.fnt",
            "GJ_button_01.png"
        ),
        [this](CCObject* sender) {
            this->onClose(sender);

            LobbyJoinPopup::create()->show();
        }
    );

    auto createBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create(
            "Create Lobby",
            "bigFont.fnt",
            "GJ_button_01.png"
        ),
        [this](CCObject* sender) {
            this->onClose(sender);

            LobbySettings defaultSettings = SwapManager::createDefaultSettings();
            LobbySettingsPopup::create(defaultSettings, [this](LobbySettings settings) {
                auto& lm = SwapManager::get();
                lm.createLobby(settings, [](std::string code) {
                    auto scene = CCScene::create();
                    scene->addChild(
                        LobbyLayer::create(code)
                    );
                    cr::utils::goToScene(scene);
                });
            })->show();
        }
    );

    mainMenu->addChild(joinBtn);
    mainMenu->addChild(createBtn);

    mainMenu->setLayout(
        ColumnLayout::create()
    );

    m_mainLayer->addChildAtPosition(mainMenu, Anchor::Center);

    return true;
}
