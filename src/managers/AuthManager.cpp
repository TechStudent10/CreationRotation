#include "AuthManager.hpp"
#include "network/packets/server.hpp"

#include <utils.hpp>

AuthManager::AuthManager() {
    auto& nm = NetworkManager::get();

    nm.on<InvalidTokenPacket>([this](InvalidTokenPacket*) {
        geode::createQuickPopup(
            "Creation Rotation",
            "The server received an <cr>invalid token</c>. Would you like to reauthenticate with the server?",
            "Cancel", "Yes",
            [this](auto, bool btn2) {
                if (!btn2) return;

                this->beginAuthorization([]() {});
            }
        );
    });

    nm.on<LoginNotReceivedPacket>([this](LoginNotReceivedPacket*) {
        geode::createQuickPopup(
            "Creation Rotation",
            "The server <cr>did not receive</c> login information. Would you like to send it now?",
            "Cancel", "Yes",
            [this](auto, bool btn2) {
                if (!btn2) return;

                this->login();
            }
        );
    });
}

void AuthManager::beginAuthorization(std::function<void()> callback) {
    auto& nm = NetworkManager::get();
    nm.send(
        RequestAuthorizationPacket::create(
            cr::utils::createAccountType().accountID
        )
    );
    nm.on<ReceiveAuthCodePacket>([this](ReceiveAuthCodePacket* packet) {
        auto glm = GameLevelManager::sharedState();
        glm->m_uploadMessageDelegate = this;
        glm->uploadUserMessage(
            packet->botAccID,
            packet->code,
            "Creation Rotation Identity Verification. This message can be safely deleted."
        );
    });
    nm.on<ReceiveTokenPacket>([this, callback](ReceiveTokenPacket* packet) {
        this->setToken(packet->token);
        this->login(callback);
    });
}

void AuthManager::login(std::function<void()> callback) {
    if (this->getToken() == "") {
        this->beginAuthorization(callback);
        return;
    }

    auto& nm = NetworkManager::get();
    nm.send(
        LoginPacket::create(this->getToken())
    );
    nm.on<LoggedInPacket>([callback](LoggedInPacket*) {
        callback();
    });
}

void AuthManager::uploadMessageFinished(int) {
    log::info("message sent");
    auto& nm = NetworkManager::get();
    nm.send(VerifyAuthPacket::create(cr::utils::createAccountType()));
}

void AuthManager::uploadMessageFailed(int) {
    log::info("message failed :(");
}
