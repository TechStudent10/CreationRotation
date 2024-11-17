#include "ChatPanel.hpp"
#include <network/manager.hpp>
#include <network/packets/all.hpp>

ChatPanel* ChatPanel::create() {
    auto ret = new ChatPanel;
    if (ret->initAnchored(350.f, 280.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ChatPanel::initialize() {
    // initialize the listeners n' stuff if they havent already
    if (!ChatPanel::hasInitialized) {
        auto& nm = NetworkManager::get();
        nm.on<MessageSentPacket>([](MessageSentPacket* packet) {
            ChatPanel::messages.push_back(packet->message);
            ChatPanel::messagesQueue.push_back(packet->message);
        });

        ChatPanel::hasInitialized = true;
    }
}

bool ChatPanel::setup() {
    this->setTitle("Chat");

    ChatPanel::initialize();

    auto scrollContainer = CCScale9Sprite::create("square02b_001.png");
    scrollContainer->setContentSize({
        m_mainLayer->getContentWidth() - 50.f,
        m_mainLayer->getContentHeight() - 85.f
    });
    scrollContainer->setColor(ccc3(0,0,0));
    scrollContainer->setOpacity(75);

    scrollLayer = ScrollLayer::create(scrollContainer->getContentSize() - 10.f);
    scrollLayer->ignoreAnchorPointForPosition(false);
    // stolen from https://github.com/HJfod/LevelTrashcan/blob/main/src/TrashcanPopup.cpp#L41
    scrollLayer->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAutoGrowAxis(scrollLayer->getContentHeight())
            ->setAxisAlignment(AxisAlignment::End)
            ->setGap(0)
    );
    scrollContainer->addChildAtPosition(scrollLayer, Anchor::Center);

    m_mainLayer->addChildAtPosition(scrollContainer, Anchor::Center, ccp(0, 5.f));

    auto inputContainer = CCNode::create();
    inputContainer->setContentSize({
        scrollContainer->getContentWidth(),
        75.f
    });
    inputContainer->setAnchorPoint({
        0.5f, 0.f
    });

    messageInput = TextInput::create(inputContainer->getContentWidth(), "Send a message to the lobby!", "chatFont.fnt");

    auto sendMsgBtn = CCMenuItemExt::createSpriteExtraWithFrameName(
        "GJ_chatBtn_001.png",
        0.75f,
        [this](CCObject*) {
            this->sendMessage();
        }
    );
    sendMsgBtn->ignoreAnchorPointForPosition(true);

    auto msgBtnMenu = CCMenu::create();
    msgBtnMenu->addChild(sendMsgBtn);
    msgBtnMenu->setContentSize(sendMsgBtn->getContentSize());

    inputContainer->addChild(messageInput);
    inputContainer->addChild(msgBtnMenu);

    inputContainer->setLayout(
        RowLayout::create()
    );
    m_mainLayer->addChildAtPosition(inputContainer, Anchor::Bottom, ccp(0, 10.f));

    for (auto message : ChatPanel::messages) {
        renderMessage(message);
    }

    this->schedule(schedule_selector(ChatPanel::updateMessages));

    return true;
}

void ChatPanel::renderMessage(Message message) {
    auto msgNode = CCNode::create();
    auto msgText = TextArea::create(
        fmt::format("<cy>{}</c>: {}", message.author.name, message.message),
        "chatFont.fnt",
        0.5f,
        scrollLayer->getContentWidth(),
        {0.f, 0.f},
        17.f,
        false
    );
    msgText->setAnchorPoint({ 0.f, 0.f });
    msgNode->setContentHeight(msgText->m_label->m_lines->count() * 17.f);
    msgNode->setContentWidth(scrollLayer->getContentWidth());
    msgText->setPosition({ 0.f, 0.f });
    msgNode->addChild(msgText);

    scrollLayer->m_contentLayer->addChild(
        msgNode
    );
    scrollLayer->m_contentLayer->updateLayout();
}

void ChatPanel::updateMessages(float dt) {
    for (auto message : ChatPanel::messagesQueue) {
        renderMessage(message);
    }
    ChatPanel::messagesQueue.clear();
}

void ChatPanel::clearMessages() {
    ChatPanel::messages.clear();

    auto& nm = NetworkManager::get();
    nm.unbind<MessageSentPacket>();
    ChatPanel::hasInitialized = false;
}

void ChatPanel::sendMessage() {
    auto msgInput = messageInput->getString();
    if (msgInput.empty()) return;
    if (geode::utils::string::replace(" ", msgInput, "") == "") return;

    auto& nm = NetworkManager::get();
    nm.send(SendMessagePacket::create(msgInput));

    messageInput->setString("");
}

void ChatPanel::keyDown(cocos2d::enumKeyCodes keycode) {
    if (keycode == cocos2d::KEY_Enter && CCIMEDispatcher::sharedDispatcher()->hasDelegate()) {
        log::debug("sending via keybind");
        sendMessage();
    } else {
        geode::Popup<>::keyDown(keycode);
    }
}
