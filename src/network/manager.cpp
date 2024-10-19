#include "manager.hpp"

NetworkManager::NetworkManager() {
    ix::initNetSystem();
}

void NetworkManager::connect(bool shouldReconnect) {
    log::debug("connecting");
    if (this->isConnected && shouldReconnect) {
        // disconnect then reconnect
        log::debug("already connected; disconnecting then reconnecting...");
        this->showDisconnectPopup = false;
        this->disconnect();
    }

    socket.setUrl(
        Mod::get()->getSettingValue<std::string>("server-url")
    );

    socket.disableAutomaticReconnection();

    socket.setPingInterval(15);

#ifdef GEODE_IS_ANDROID
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.caFile = (Mod::get()->getResourcesDir() / "cacert-2024-09-24.pem").string();
    socket.setTLSOptions(tlsOptions);
#endif

    socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Error) {
            const auto errReason = msg->errorInfo.reason;
            log::error("ixwebsocket error: {}", errReason);
            Loader::get()->queueInMainThread([this, errReason]() {
                FLAlertLayer::create(
                    "CR Error",
                    fmt::format("There was an error while connecting to the server: {}", errReason),
                    "OK"
                )->show();
            });
            return;
        } else if (msg->type == ix::WebSocketMessageType::Open) {
            log::debug("connection success!");
            // send login data
            auto loginInfo = matjson::Object({
                {"version", Mod::get()->getVersion().toVString()}
            });
            socket.send(fmt::format("login|{}", matjson::Value(loginInfo).dump(0)));
            this->showDisconnectPopup = true;
            Loader::get()->queueInMainThread([]() {
                Notification::create(
                    "Connection sucessful!",
                    NotificationIcon::Success,
                    1.5f
                )->show();
            });
            return;
        } else if (msg->type == ix::WebSocketMessageType::Close) {
            log::debug("connection closed");
            this->isConnected = false;
            const auto reason = msg->closeInfo.reason;
            Loader::get()->queueInMainThread([this, reason]() {
                Notification::create(
                    "Disconnected from Creation Rotation",
                    NotificationIcon::Error,
                    1.5f
                )->show();
                if (this->showDisconnectPopup) {
                    geode::createQuickPopup(
                        "Disconnected",
                        fmt::format("You have been disconnected from the Creation Rotation servers. Reason:\n\n{}", reason),
                        "OK", "Close",
                        [this, reason](auto, bool) {
                            if (disconnectCallback) disconnectCallback(reason);
                        }
                    );
                }
            });
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

    auto packetIdIdx = strMsg.find("|");
    if (packetIdIdx == std::string::npos) {
        log::error("invalid packet recieved");
        return;
    }

    auto packetStr = strMsg.substr(packetIdIdx + 1);
    auto packetId = geode::utils::numFromString<int>(strMsg.substr(0, packetIdIdx)).unwrapOr(0);

    if (!listeners.contains(packetId)) {
        log::error("unhandled packed ID {}", packetId);
        return;
    }

    listeners.at(packetId)(packetStr);
}
