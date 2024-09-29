#include "Lobby.hpp"
#include <managers/SwapManager.hpp>

#include <network/manager.hpp>
#include <network/packets/server.hpp>
#include <network/packets/client.hpp>

#include "LobbySettings.hpp"

#include <cvolton.level-id-api/include/EditorIDs.hpp>

PlayerCell* PlayerCell::create(std::string playerName, int mainColor, int secondColor, int glowColor, int iconID, float width) {
    auto ret = new PlayerCell;
    if (ret->init(
        playerName,
        mainColor,
        secondColor,
        glowColor,
        iconID,
        width
    )) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool PlayerCell::init(std::string playerName, int mainColor, int secondColor, int glowColor, int iconID, float width) {
    this->setContentSize({
        width,
        CELL_HEIGHT
    });

    auto player = SimplePlayer::create(0);
    auto gm = GameManager::get();

    player->updatePlayerFrame(iconID, IconType::Cube);
    player->setColor(gm->colorForIdx(mainColor));
    player->setSecondColor(gm->colorForIdx(secondColor));

    if (glowColor == -1) {
        player->disableGlowOutline();
    } else {
        player->setGlowOutline(gm->colorForIdx(glowColor));
    }

    player->setPosition({ 25.f, CELL_HEIGHT / 2.f});
    player->setAnchorPoint({ 0.5f, 0.5f });

    this->addChild(player);

    auto menu = CCMenu::create();
    auto nameBtn = CCMenuItemSpriteExtra::create(
        CCLabelBMFont::create(playerName.c_str(), "bigFont.fnt"), this, menu_selector(PlayerCell::onOpenProfile)
    );
    nameBtn->setPosition({ 0.f, 0.f });
    nameBtn->ignoreAnchorPointForPosition(true);
    menu->addChild(nameBtn);
    menu->setContentSize(
        nameBtn->getContentSize()
    );
    menu->setPosition({
        55.f, CELL_HEIGHT / 2.f
    });
    menu->setAnchorPoint({ 0.f, 0.5f });
    menu->ignoreAnchorPointForPosition(false);

    this->addChild(menu);

    return true;
}

void PlayerCell::onOpenProfile(CCObject* sender) {}

LobbyLayer* LobbyLayer::create(std::string code) {
    auto ret = new LobbyLayer();
    if (ret->init(code)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LobbyLayer::init(std::string code) {
    lobbyCode = code;

    auto director = CCDirector::sharedDirector();
    auto size = director->getWinSize();

    mainLayer = CCLayer::create();
    mainLayer->setContentSize(size);

    background = CCSprite::create("GJ_gradientBG.png");
    background->setScaleX(
        size.width / background->getContentWidth()
    );
    background->setScaleY(
        size.height / background->getContentHeight()
    );
    background->setAnchorPoint({ 0, 0 });
    background->setColor({ 0, 102, 255 });
    background->setZOrder(-10);
    mainLayer->addChild(background);

    auto closeBtnSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    closeBtn = CCMenuItemSpriteExtra::create(
        closeBtnSprite, this, menu_selector(LobbyLayer::onClose)
    );
    auto closeMenu = CCMenu::create();
    closeMenu->addChild(closeBtn);
    closeMenu->setPosition({ 25, size.height - 25 });
    mainLayer->addChild(closeMenu);

    geode::addSideArt(mainLayer, SideArt::Bottom);
    geode::addSideArt(mainLayer, SideArt::TopRight);

    auto startBtnSpr = CCSprite::create("swap-btn.png"_spr);
    startBtnSpr->setScale(0.3f);
    startBtn = CCMenuItemSpriteExtra::create(
        startBtnSpr, this, menu_selector(LobbyLayer::onStart)
    );
    auto startMenu = CCMenu::create();
    startMenu->setZOrder(5);
    startMenu->addChild(startBtn);
    startMenu->setPosition({
        size.width / 2, 30.f
    });

    mainLayer->addChild(startMenu);

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
    settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr, this, menu_selector(LobbyLayer::onSettings)
    );

    auto refreshBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
        "GJ_getSongInfoBtn_001.png",
        1.f,
        [this](CCObject* sender) {
            SwapManager::get().getLobbyInfo([this](LobbyInfo info) {
                refresh(info); 
            });
        }
    );
    auto bottomMenu = CCMenu::create();
    bottomMenu->addChild(settingsBtn);
    bottomMenu->addChild(refreshBtn);
    bottomMenu->setPosition(
        size.width - 25.f, bottomMenu->getChildrenCount() * 25.f
    );

    bottomMenu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
    );

    this->setKeyboardEnabled(true);
    this->setKeypadEnabled(true);
    this->addChild(mainLayer);
    this->addChild(bottomMenu);

    SwapManager::get().getLobbyInfo([this](LobbyInfo info) {
        refresh(info); 
    });
    registerListeners();

    log::debug("LobbyLayer setup!");

    return true;
}

void LobbyLayer::registerListeners() {
    auto& nm = NetworkManager::get();
    nm.on<LobbyUpdatedPacket>([this](LobbyUpdatedPacket* packet) {
        this->refresh(packet->info);
    });
    nm.on<SwapStartedPacket>([this](SwapStartedPacket* packet) {
        auto& sm = SwapManager::get();
        sm.startSwap(packet);
    });
}

void LobbyLayer::unregisterListeners() {
    auto& nm = NetworkManager::get();
    nm.unbind<LobbyUpdatedPacket>();
    nm.unbind<SwapStartedPacket>();
}

LobbyLayer::~LobbyLayer() {
    unregisterListeners();
}

void LobbyLayer::refresh(LobbyInfo info) {
    isOwner = GameManager::get()->m_playerUserID == info.settings.owner;

    loadingCircle = LoadingCircle::create();
    loadingCircle->setParentLayer(this);
    loadingCircle->setFade(true);
    loadingCircle->show();

    auto size = CCDirector::sharedDirector()->getWinSize();
    auto listWidth = size.width / 1.5f;

    if (!titleLabel) {
        titleLabel = CCLabelBMFont::create(
            info.settings.name.c_str(),
            "bigFont.fnt"
        );
        titleLabel->limitLabelWidth(300.f, 1.f, 0.1f);

        auto menu = CCMenu::create();
        menu->setPosition({
            size.width / 2, size.height - 25
        });

        menu->addChild(
            CCMenuItemExt::createSpriteExtra(
                titleLabel,
                [info](CCObject* sender) {
                    geode::utils::clipboard::write(
                        info.code
                    );
                    Notification::create("Copied code to clipboard")->show();
                }
            )
        );

        mainLayer->addChild(menu);
    }

    titleLabel->setString(fmt::format("{} ({})", info.settings.name, info.code).c_str());

    if (playerList) playerList->removeFromParent();

    using namespace geode::utils; 
    playerListItems = CCArray::create();
    for (auto acc : info.accounts) {
        playerListItems->addObject(
            PlayerCell::create(
                acc.name,
                numFromString<int>(acc.color1).unwrapOr(1),
                numFromString<int>(acc.color2).unwrapOr(3),
                numFromString<int>(acc.color3).unwrapOr(-1),
                numFromString<int>(acc.iconID).unwrapOr(1),
                listWidth
            )
        );
    }
    playerList = ListView::create(playerListItems, PlayerCell::CELL_HEIGHT, listWidth);
    playerList->setPosition({ size.width / 2, size.height / 2 - 10.f });
    playerList->setAnchorPoint({ 0.5f, 0.5f });
    playerList->ignoreAnchorPointForPosition(false);

    createBorders();

    mainLayer->addChild(playerList);

    if (!mainLayer->getChildByIDRecursive("list-bg")) {
        auto listBG = CCLayerColor::create({ 0, 0, 0, 85 });
        listBG->ignoreAnchorPointForPosition(false);
        listBG->setAnchorPoint({ 0.5f, 0.5f });
        listBG->setPosition(
            playerList->getPosition()
        );
        listBG->setZOrder(-1);
        listBG->setID("list-bg");
        listBG->setContentSize(playerList->getContentSize());
        mainLayer->addChild(listBG);
    }

    settingsBtn->setVisible(isOwner);
    startBtn->setVisible(isOwner);

    loadingCircle->fadeAndRemove();
}

void LobbyLayer::onStart(CCObject* sender) {
    if (!isOwner) return;

    auto& nm = NetworkManager::get();
    nm.send(StartSwapPacket::create(lobbyCode));
}

void LobbyLayer::onSettings(CCObject* sender) {
    auto& lm = SwapManager::get();
    lm.getLobbyInfo([this](LobbyInfo info) {
        auto settingsPopup = LobbySettingsPopup::create(info.settings, [this](LobbySettings settings) {
            auto& lm = SwapManager::get();
            lm.updateLobby(settings);
        });
        settingsPopup->show();
    });
}

void LobbyLayer::createBorders() {
    // SIDES //

    #define CREATE_SIDE() CCSprite::createWithSpriteFrameName("GJ_table_side_001.png")
    
    const int SIDE_OFFSET = 7;

    // TOP //

    auto topSide = CREATE_SIDE();
    topSide->setScaleY(
        playerList->getContentWidth() / topSide->getContentHeight()
    );
    topSide->setRotation(90.f);
    topSide->setPosition({
        playerList->m_width / 2,
        playerList->m_height
    });
    topSide->setID("top-border");
    topSide->setZOrder(3);

    // BOTTOM //

    auto bottomSide = CREATE_SIDE();
    bottomSide->setScaleY(
        playerList->getContentWidth() / bottomSide->getContentHeight()
    );
    bottomSide->setRotation(-90.f);
    bottomSide->setPosition({
        playerList->m_width / 2,
        0
    });
    bottomSide->setID("bottom-border");
    bottomSide->setZOrder(3);

    // LEFT //

    auto leftSide = CREATE_SIDE();
    leftSide->setScaleY(
        playerList->getContentHeight() / leftSide->getContentHeight()
    );
    leftSide->setPosition({
        -SIDE_OFFSET,
        playerList->m_height / 2
    });
    leftSide->setID("left-border");

    // RIGHT //

    auto rightSide = CREATE_SIDE();
    rightSide->setScaleY(
        playerList->getContentHeight() / rightSide->getContentHeight()
    );
    rightSide->setRotation(180.f);
    rightSide->setPosition({
        playerList->m_width + SIDE_OFFSET,
        playerList->m_height / 2
    });
    rightSide->setID("right-border");

    playerList->addChild(topSide);
    playerList->addChild(bottomSide);
    playerList->addChild(leftSide);
    playerList->addChild(rightSide);

    // CORNERS //

    #define CREATE_CORNER() CCSprite::createWithSpriteFrameName("GJ_table_corner_001.png")

    // TOP-LEFT //
    
    auto topLeftCorner = CREATE_CORNER();
    topLeftCorner->setPosition({
        leftSide->getPositionX(), topSide->getPositionY()
    });
    topLeftCorner->setZOrder(2);
    topLeftCorner->setID("top-left-corner");

    // TOP-RIGHT //
    auto topRightCorner = CREATE_CORNER();
    topRightCorner->setFlipX(true);
    topRightCorner->setPosition({
        rightSide->getPositionX(), topSide->getPositionY()
    });
    topRightCorner->setZOrder(2);
    topRightCorner->setID("top-right-corner");

    // BOTTOM-LEFT //
    
    auto bottomLeftCorner = CREATE_CORNER();
    bottomLeftCorner->setFlipY(true);
    bottomLeftCorner->setPosition({
        leftSide->getPositionX(), bottomSide->getPositionY()
    });
    bottomLeftCorner->setZOrder(2);
    bottomLeftCorner->setID("bottom-left-corner");

    // BOTTOM-RIGHT //
    auto bottomRightCorner = CREATE_CORNER();
    bottomRightCorner->setFlipX(true);
    bottomRightCorner->setFlipY(true);
    bottomRightCorner->setPosition({
        rightSide->getPositionX(), bottomSide->getPositionY()
    });
    bottomRightCorner->setZOrder(2);
    bottomRightCorner->setID("bottom-right-corner");

    playerList->addChild(topLeftCorner);
    playerList->addChild(topRightCorner);
    playerList->addChild(bottomLeftCorner);
    playerList->addChild(bottomRightCorner);
}

void LobbyLayer::onClose(CCObject* sender) {
    keyBackClicked();
}

void LobbyLayer::keyBackClicked() {
    geode::createQuickPopup(
        "Disconnect",
        "Are you want to <cr>disconnect</c> from the <cy>lobby</c> and <cy>server</c>?",
        "Cancel", "Yes",
        [this](auto, bool btn2) {
            if (!btn2) return;

            auto& lm = SwapManager::get();
            lm.disconnectLobby();

            cr::utils::popScene();
        }
    );
}
