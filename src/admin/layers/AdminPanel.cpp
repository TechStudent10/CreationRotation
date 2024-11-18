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

    auto accountIDInput = TextInput::create(100.f, "Account ID");
    auto reasonInput = TextInput::create(100.f, "Reason");

    auto banSpr = ButtonSprite::create("Banish to the shadow realm!");
    banSpr->setScale(0.5f);
    auto banBtn = CCMenuItemExt::createSpriteExtra(
        banSpr,
        [accountIDInput, reasonInput](CCObject*) {
            auto& nm = NetworkManager::get();
            nm.send(BanUserPacket::create(
                geode::utils::numFromString<int>(accountIDInput->getString()).unwrapOr(0),
                reasonInput->getString()
            ));
        }
    );

    auto unbanSpr = ButtonSprite::create("ARISE!");
    unbanSpr->setScale(0.5f);
    auto unbanBtn = CCMenuItemExt::createSpriteExtra(
        unbanSpr,
        [accountIDInput](CCObject*) {
            auto& nm = NetworkManager::get();
            nm.send(UnbanUserPacket::create(
                geode::utils::numFromString<int>(accountIDInput->getString()).unwrapOr(0)
            ));
        }
    );

    auto primaryNode = CCNode::create();

    primaryNode->setAnchorPoint({ 0.5f, 0.5f });
    primaryNode->addChild(accountIDInput);
    primaryNode->addChild(reasonInput);

    primaryNode->setContentHeight(accountIDInput->getContentHeight() * 2.f);

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
