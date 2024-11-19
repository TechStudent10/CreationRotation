#include "AdminPanel.hpp"

#include <network/manager.hpp>
#include <network/packets/all.hpp>

AdminPanel* AdminPanel::create() {
    auto ret = new AdminPanel;
    if (ret->initAnchored(220.f, 265.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AdminPanel::setup() {
    this->setTitle("Administrator Panel");

    auto usernameInput = TextInput::create(100.f, "Username");
    auto reasonInput = TextInput::create(100.f, "Reason");

    auto banSpr = ButtonSprite::create("Ban");
    banSpr->setScale(0.5f);
    auto banBtn = CCMenuItemExt::createSpriteExtra(
        banSpr,
        [usernameInput, reasonInput](CCObject*) {
            auto& nm = NetworkManager::get();
            nm.send(BanUserPacket::create(
                usernameInput->getString(),
                reasonInput->getString()
            ));
        }
    );

    auto unbanSpr = ButtonSprite::create("Unban");
    unbanSpr->setScale(0.5f);
    auto unbanBtn = CCMenuItemExt::createSpriteExtra(
        unbanSpr,
        [usernameInput](CCObject*) {
            auto& nm = NetworkManager::get();
            nm.send(UnbanUserPacket::create(
                geode::utils::numFromString<int>(usernameInput->getString()).unwrapOr(0)
            ));
        }
    );

    auto primaryNode = CCNode::create();

    primaryNode->setAnchorPoint({ 0.5f, 0.5f });
    primaryNode->addChild(usernameInput);
    primaryNode->addChild(reasonInput);

    primaryNode->setContentHeight(usernameInput->getContentHeight() * 2.f);

    primaryNode->setLayout(
        ColumnLayout::create()
    );

    m_mainLayer->addChildAtPosition(primaryNode, Anchor::Center);

    auto banMenu = CCMenu::create();
    banMenu->addChild(unbanBtn);
    banMenu->addChild(banBtn);
    banMenu->setLayout(
        ColumnLayout::create()
    );

    m_mainLayer->addChildAtPosition(banMenu, Anchor::Bottom, ccp(0, 10.f));

    return true;
}
