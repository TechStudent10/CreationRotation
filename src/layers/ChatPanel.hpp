#pragma once
#include <Geode/Geode.hpp>
#include <types/lobby.hpp>

using namespace geode::prelude;

class ChatPanel : public geode::Popup {
protected:
    float m_width;
    float m_height;

    inline static bool hasInitialized = false;
    inline static std::vector<Message> messages = {};
    inline static std::vector<Message> messagesQueue = {};

    ScrollLayer* scrollLayer = nullptr;
    TextInput* messageInput = nullptr;

    bool init() override;

    void sendMessage();
    void renderMessage(Message const& message);
    void updateMessages(float dt);

    void keyDown(cocos2d::enumKeyCodes, double) override;
public:
    static ChatPanel* create();

    static void clearMessages();
    static void initialize();
};
