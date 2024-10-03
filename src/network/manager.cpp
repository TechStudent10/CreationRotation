#include "manager.hpp"

NetworkManager::NetworkManager() {
    ix::initNetSystem();
}

void NetworkManager::connect() {
    if (this->isConnected) return; // why are you connecting twice

    socket.setUrl(
        Mod::get()->getSettingValue<std::string>("server-url")
    );

    socket.disableAutomaticReconnection();

    socket.setPingInterval(15);

    socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Error) {
            log::error("ixwebsocket error: {}", msg->errorInfo.reason);
            FLAlertLayer::create(
                "CR Error",
                fmt::format("There was an error while connecting to the server: {}", msg->errorInfo.reason),
                "OK"
            )->show();
            return;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            log::debug("connection success!");
            Notification::create(
                "Connection sucessful!",
                NotificationIcon::Success,
                1.5f
            )->show();
            return;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            log::debug("connection closed");
            Notification::create(
                "Disconnected from Creation Rotation",
                NotificationIcon::Error,
                1.5f
            )->show();
            return;
        } else if (msg->type == ix::WebSocketMessageType::Message) {
            if (msg.get()->str == "") return;
            this->onMessage(msg);
        }
    });

    socket.start();

    this->isConnected = true;
}

void NetworkManager::disconnect() {
    this->socket.stop();
    this->isConnected = false;
}

void NetworkManager::onMessage(const ix::WebSocketMessagePtr& msg) {
    auto strMsg = msg.get()->str;
    log::debug("raw string: {}", strMsg);

    auto packetIdIdx = strMsg.find("|");
    if (packetIdIdx == std::string::npos) {
        log::error("invalid packet recieved");
        return;
    }

    auto packetStr = strMsg.substr(packetIdIdx + 1);
    auto packetId = geode::utils::numFromString<int>(strMsg.substr(0, packetIdIdx)).unwrapOr(0);

    log::debug("packet ID: {}", packetId);
    log::debug("packet Str: {}", packetStr);

    if (!listeners.contains(packetId)) {
        log::error("unhandled packed ID {}", packetId);
        return;
    }

    log::debug("recieved and handling packet {}", packetId);

    listeners.at(packetId)(packetStr);
}
