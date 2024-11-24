#include "LobbyJoinPopup.hpp"

#include "Lobby.hpp"
#include <managers/SwapManager.hpp>
#include <utils.hpp>

LobbyJoinPopup* LobbyJoinPopup::create() {
    auto ret = new LobbyJoinPopup;
    if (ret->initAnchored(140.f, 140.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LobbyJoinPopup::setup() {
    m_noElasticity = true;
    this->setTitle("Join Lobby");

    auto label = CCLabelBMFont::create(
        "Lobby Code",
        "bigFont.fnt"
    );
    label->setScale(0.45f);

    auto input = TextInput::create(100.f, "Code");
    input->setPasswordMode(
        Mod::get()->getSettingValue<bool>("hide-code")
    );

    auto submitBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Submit"),
        [this, input](CCObject* sender) {
            this->onClose(sender);

            auto& lm = SwapManager::get();
            auto code = input->getString();
            lm.joinLobby(code, [code]() {
                auto scene = CCScene::create();
                scene->addChild(
                    LobbyLayer::create(code)
                );
                cr::utils::goToScene(scene);
            });
        }
    );

    auto submitMenu = CCMenu::create();
    submitMenu->addChild(submitBtn);

    m_mainLayer->addChildAtPosition(input, Anchor::Center);
    m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0, 25.f));
    m_mainLayer->addChildAtPosition(submitMenu, Anchor::Bottom, ccp(0, 25.f));

    return true;
}
