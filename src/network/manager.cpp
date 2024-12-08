#include "manager.hpp"

#include <utils.hpp>

#include "packets/client.hpp"
#include "packets/server.hpp"

time_t lastPinged = 0;

NetworkManager::NetworkManager() {
    ix::initNetSystem();

    // schedule pinging
    CCScheduler::get()->scheduleUpdateForTarget(this, -1, false);

    this->on<PongPacket>([this](PongPacket*) {
        responseTime = time(0) - lastPinged;
    });
}

void NetworkManager::update(float dt) {
    if (!this->isConnected) return;

    if (time(0) - lastPinged < 10) return;
    lastPinged = time(0);

    this->send(PingPacket::create());
}

void NetworkManager::connect(bool shouldReconnect, std::function<void()> callback) {
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

    // socket.setPingInterval(15);

#ifdef GEODE_IS_ANDROID
    ix::SocketTLSOptions tlsOptions;
    tlsOptions.caFile = (Mod::get()->getResourcesDir() / "cacert-2024-09-24.pem").string();
    socket.setTLSOptions(tlsOptions);
#endif

    socket.setOnMessageCallback([this, callback](const ix::WebSocketMessagePtr& msg) {
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

            // call middleware (this should run the provided cb)
            middleware([this, callback]() {
                // register error packet
                this->on<ErrorPacket>([](ErrorPacket* packet) {
                    FLAlertLayer::create(
                        "CR Error",
                        fmt::format("The Creation Rotation server sent an error: <cy>{}</c>", packet->error).c_str(),
                        "OK"
                    )->show();
                });

                // call the callback
                callback();

                // send all packets in queue
                for (auto packetFn : packetQueue) {
                    packetFn();
                }

                // clear the queue
                packetQueue.clear();

                this->showDisconnectPopup = true;
                Loader::get()->queueInMainThread([]() {
                    Notification::create(
                        "Connection sucessful!",
                        NotificationIcon::Success,
                        1.5f
                    )->show();
                });
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
                if (disconnectEventCb) disconnectEventCb(reason);
                if (this->showDisconnectPopup) {
                    geode::createQuickPopup(
                        "Disconnected",
                        fmt::format("You have been disconnected from the Creation Rotation servers. Reason:\n\n{}", reason),
                        "OK", "Close",
                        [this, reason](auto, bool) {
                            if (disconnectBtnCallback) disconnectBtnCallback(reason);
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
    
    this->isConnected = true;
    socket.start();
}

void NetworkManager::disconnect() {
    this->socket.stop();
    this->isConnected = false;
}

void NetworkManager::onMessage(const ix::WebSocketMessagePtr& msg) {
    auto strMsg = msg.get()->str;

    auto packetIdIdx = strMsg.find("|");
    if (packetIdIdx == std::string::npos) {
        log::error("invalid packet received");
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
