#include "AuthPopup.hpp"
#include "AdminPanel.hpp"

#include <network/manager.hpp>
#include <network/packets/client.hpp>
#include <network/packets/server.hpp>

#include <utils.hpp>

AuthPopup* AuthPopup::create() {
    auto ret = new AuthPopup;
    if (ret->initAnchored(140.f, 140.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AuthPopup::setup() {
    m_noElasticity = true;
    this->setTitle("Admin Authorization");

    auto& nm = NetworkManager::get();

    auto label = CCLabelBMFont::create(
        "Password",
        "bigFont.fnt"
    );
    label->setScale(0.45f);

    auto input = TextInput::create(100.f, "Passwd");
    input->setPasswordMode(true);

    auto submitBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Login"),
        [this, input](CCObject* sender) {
            this->onClose(sender);
            auto& nm = NetworkManager::get();
            nm.send(AuthorizeUserPacket::create(input->getString()));
        }
    );

    nm.on<AuthorizedUserPacket>([this](AuthorizedUserPacket*) {
        AdminPanel::create()->show();
    });

    auto submitMenu = CCMenu::create();
    submitMenu->addChild(submitBtn);

    m_mainLayer->addChildAtPosition(input, Anchor::Center);
    m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0, 25.f));
    m_mainLayer->addChildAtPosition(submitMenu, Anchor::Bottom, ccp(0, 25.f));

    return true;
}
