#include "LobbyJoinPopup.hpp"

#include "Lobby.hpp"
#include <managers/SwapManager.hpp>
#include <utils.hpp>

LobbyJoinPopup* LobbyJoinPopup::create() {
    auto ret = new LobbyJoinPopup;
    if (ret->initAnchored(280.f, 180.f)) {
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
        "Code",
        "bigFont.fnt"
    );
    label->setScale(0.45f);

    auto input = TextInput::create(100.f, "Code");

    auto submitBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Submit"),
        [this, input](CCObject* sender) {
            this->onClose(sender);

            auto& lm = SwapManager::get();
            lm.joinLobby(input->getString(), [input]() {
                auto scene = CCScene::create();
                scene->addChild(
                    LobbyLayer::create(input->getString())
                );
                cr::utils::goToScene(scene);
            });
        }
    );

    auto submitMenu = CCMenu::create();
    submitMenu->addChild(submitBtn);

    m_mainLayer->addChildAtPosition(input, Anchor::Center);
    m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0, -15.f));
    m_mainLayer->addChildAtPosition(submitMenu, Anchor::Bottom);

    return true;
}
