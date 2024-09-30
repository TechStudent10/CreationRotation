#include "manager.hpp"

NetworkManager::NetworkManager() {}

void NetworkManager::connect() {
    if (this->isConnected) return; // why are you connecting twice

    // client = new sio::client();
    // client->connect("http://127.0.0.1:3000"); // TODO: make this not localhost

    socket.setUrl(
        Mod::get()->getSettingValue<std::string>("server-url")
    );

    socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        log::info("message lol");
        if (msg.get()->str == "") return;

        this->onMessage(msg);
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

// const sio::socket::ptr& NetworkManager::getSocket(std::string nspace) {
//     return client->socket(nspace);
// }
