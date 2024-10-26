#include "LobbySelectPopup.hpp"

#include <managers/SwapManager.hpp>
#include <network/manager.hpp>

#include <layers/Lobby.hpp>
#include <layers/LobbySettings.hpp>
#include <layers/LobbyJoinPopup.hpp>

#include <utils.hpp>
#include <types/lobby.hpp>

#include <network/packets/client.hpp>
#include <network/packets/server.hpp>

class LobbyItem : public CCLayer {
protected:
    bool init(float width, LobbyInfo lobby, std::function<void()> callback) {
        if (!CCLayer::init()) return false;

        auto nameLabel = CCLabelBMFont::create(
            lobby.settings.name.c_str(),
            "bigFont.fnt"
        );
        nameLabel->setPosition({ 10.f, CELL_HEIGHT / 2.f + 5.f });
        nameLabel->setAnchorPoint({
            0.f, 0.5f
        });
        nameLabel->setScale(0.5f);

        auto authorLabel = CCLabelBMFont::create(
            fmt::format("By {}", lobby.settings.owner.name).c_str(),
            "goldFont.fnt"
        );
        authorLabel->setPosition({ 10.f, CELL_HEIGHT / 2.f - 5.f });
        authorLabel->setAnchorPoint({
            0.f, 0.5f
        });
        authorLabel->setScale(0.5f);

        auto joinSpr = ButtonSprite::create(
            "Join",
            "bigFont.fnt",
            "GJ_button_01.png"
        );
        joinSpr->setScale(0.7f);
        auto joinBtn = CCMenuItemExt::createSpriteExtra(
            joinSpr,
            [lobby, callback](CCObject*) {
                auto& lm = SwapManager::get();
                lm.joinLobby(lobby.code, [lobby, callback]() {
                    callback();
                    auto scene = CCScene::create();
                    scene->addChild(
                        LobbyLayer::create(lobby.code)
                    );
                    cr::utils::goToScene(scene);
                });
            }
        );
        auto joinMenu = CCMenu::create();
        joinMenu->addChild(joinBtn);
        joinMenu->setPosition({ width - joinBtn->getScaledContentWidth() + 15.f, CELL_HEIGHT / 2.f });
        
        this->addChild(nameLabel);
        this->addChild(authorLabel);
        this->addChild(joinMenu);

        this->setContentSize({ width, CELL_HEIGHT });

        return true;
    }
public:
    static constexpr float CELL_HEIGHT = 40.f;

    static LobbyItem* create(float width, LobbyInfo lobby, std::function<void()> callback) {
        auto ret = new LobbyItem;
        if (ret->init(width, lobby, callback)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

LobbySelectPopup* LobbySelectPopup::create() {
    auto ret = new LobbySelectPopup;
    if (ret->initAnchored(370.f, 265.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LobbySelectPopup::setup() {
    this->setTitle("Creation Rotation");

    auto mainMenu = CCMenu::create();

    auto joinSpr = ButtonSprite::create(
        "Join Lobby",
        "bigFont.fnt",
        "GJ_button_01.png"
    );
    joinSpr->setScale(0.75f);
    auto joinBtn = CCMenuItemExt::createSpriteExtra(
        joinSpr,
        [this](CCObject* sender) {
            this->onClose(sender);

            LobbyJoinPopup::create()->show();
        }
    );

    auto createSpr = ButtonSprite::create(
        "Create Lobby",
        "bigFont.fnt",
        "GJ_button_01.png"
    );
    createSpr->setScale(0.75f);
    auto createBtn = CCMenuItemExt::createSpriteExtra(
        createSpr,
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
        RowLayout::create()
            ->setAxisReverse(true)
    );

    m_mainLayer->addChildAtPosition(mainMenu, Anchor::Bottom, ccp(0, 25.f));

    auto& nm = NetworkManager::get();
    nm.setDisconnectCallback([this](std::string reason) {
        if (this->m_closeBtn) return;
        this->m_closeBtn->activate();
    });

    // lobbyList = ListView::create(CCArray::create(), LobbyItem::CELL_HEIGHT);
    refresh(true);

    return true;
}

void LobbySelectPopup::refresh(bool isFirstTime) {
    auto& nm = NetworkManager::get();
    nm.addToQueue(GetPublicLobbiesPacket::create());
    nm.on<RecievePublicLobbiesPacket>([this, isFirstTime](RecievePublicLobbiesPacket* packet) {
        if (!isFirstTime) lobbyList->removeFromParent();
        auto listItems = CCArray::create();

        for (auto lobby : packet->lobbies) {
            listItems->addObject(
                LobbyItem::create(350.f, lobby, [this]() { this->m_closeBtn->activate(); })
            );
        }

        auto list = ListView::create(listItems, LobbyItem::CELL_HEIGHT, 350.f, 170.f);
        list->ignoreAnchorPointForPosition(false);

        lobbyList = Border::create(
            list,
            {0, 0, 0, 75},
            {350.f, 170.f}
        );
        if (auto borderSprite = typeinfo_cast<CCScale9Sprite*>(lobbyList->getChildByID("geode.loader/border_sprite"))) {
            float scaleFactor = 1.7f;
            borderSprite->setContentSize(CCSize{borderSprite->getContentSize().width, borderSprite->getContentSize().height + 3} / scaleFactor);
            borderSprite->setScale(scaleFactor);
            borderSprite->setPositionY(-0.5);
        }
        lobbyList->ignoreAnchorPointForPosition(false);

        m_mainLayer->addChildAtPosition(lobbyList, Anchor::Center, ccp(0.f, 10.f));
    }, true);
}
