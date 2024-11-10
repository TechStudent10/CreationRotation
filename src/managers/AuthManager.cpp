#include "AuthManager.hpp"

#include <utils.hpp>

void AuthManager::beginAuthorization(std::function<void()> callback) {
    auto& nm = NetworkManager::get();
    nm.send(
        RequestAuthorizationPacket::create(
            cr::utils::createAccountType().accountID
        )
    );
    nm.on<RecieveAuthCodePacket>([this](RecieveAuthCodePacket* packet) {
        auto glm = GameLevelManager::sharedState();
        glm->m_uploadMessageDelegate = this;
        glm->uploadUserMessage(
            packet->botAccID,
            packet->code,
            "Creation Rotation Identity Verification. This message can be safely deleted."
        );
    });
    nm.on<RecieveTokenPacket>([this, callback](RecieveTokenPacket* packet) {
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
    Loader::get()->queueInMainThread([callback]() {
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
