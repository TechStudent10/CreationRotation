#include <Geode/Geode.hpp>
#include "SwapManager.hpp"

#include <network/manager.hpp>
#include <network/packets/client.hpp>
#include <network/packets/server.hpp>

#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <hjfod.gmd-api/include/GMD.hpp>

#include <utils.hpp>

using namespace geode::prelude;

#define CR_REQUIRE_CONNECTION() if(!NetworkManager::get().isConnected) NetworkManager::get().connect();

SwapManager::SwapManager() {
    CR_REQUIRE_CONNECTION()
    auto& nm = NetworkManager::get();
    nm.on<ErrorPacket>([](ErrorPacket* packet) {
        FLAlertLayer::create(
            "CR Error",
            fmt::format("The Creation Rotation server sent an error: <cy>{}</c>", packet->errorStr).c_str(),
            "OK"
        )->show();
    });
}

LobbySettings SwapManager::createDefaultSettings() {
    auto acc = SwapManager::createAccountType();
    log::info("{}", acc.userID);

    return {
        .name = fmt::format("{}'s Lobby", acc.name),
        .turns = 4,
        .owner = geode::utils::numFromString<int>(acc.userID).unwrapOr(0),
        .minutesPerTurn = 5
    };
}

void SwapManager::createLobby(LobbySettings lobby, std::function<void(std::string)> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();
    nm.send(CreateLobbyPacket::create(lobby));
    nm.on<LobbyCreatedPacket>([this, callback](LobbyCreatedPacket* createdLobby) {
        this->joinLobby(createdLobby->info.code, [createdLobby, callback]() {
            callback(createdLobby->info.code);
        });
    });
}

void SwapManager::joinLobby(std::string code, std::function<void()> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();

    nm.send(JoinLobbyPacket::create(code, this->createAccountType()));

    nm.on<JoinedLobbyPacket>([this, callback, code](auto) {
        this->currentLobbyCode = code;
        callback();
    }, "", true);
}

Account SwapManager::createAccountType() {
    auto gm = GameManager::get();

    return {
        .name = gm->m_playerName,
        .userID = std::to_string(gm->m_playerUserID.value()),
        .iconID = std::to_string(gm->getPlayerFrame()),
        .color1 = std::to_string(gm->m_playerColor.value()),
        .color2 = std::to_string(gm->m_playerColor2.value()),
        .color3 = gm->m_playerGlow ?
            std::to_string(gm->m_playerGlowColor.value()) :
            "-1"
    };
}

void SwapManager::getLobbyAccounts(std::function<void(std::vector<Account>)> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();
    nm.send(GetAccountsPacket::create(this->currentLobbyCode));
    nm.on<RecieveAccountsPacket>([this, callback](RecieveAccountsPacket* accounts) {
        callback(accounts->accounts);
    });
}

void SwapManager::getLobbyInfo(std::function<void(LobbyInfo)> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();
    nm.send(GetLobbyInfoPacket::create(this->currentLobbyCode));
    nm.on<RecieveLobbyInfoPacket>([this, callback](RecieveLobbyInfoPacket* packet) {
        callback(packet->info);
    });
}

void SwapManager::updateLobby(LobbySettings updatedLobby) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();

    nm.send(UpdateLobbyPacket::create(this->currentLobbyCode, updatedLobby));
}

void SwapManager::disconnectLobby() {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();

    nm.send(DisconnectFromLobbyPacket::create());
    nm.disconnect();
}

// LEVEL SWAP //

void SwapManager::registerListeners() {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();

    auto nspace = fmt::format("/{}", currentLobbyCode);
    nm.on<TimeToSwapPacket>([this, nspace](TimeToSwapPacket* p) {
        auto& nm = NetworkManager::get();

        Notification::create("Swapping levels!", NotificationIcon::Info, 1.5f)->show();

        auto filePath = std::filesystem::temp_directory_path() / fmt::format("temp{}.gmd", rand());

        if (auto editorLayer = LevelEditorLayer::get()) {
            auto fakePauseLayer = EditorPauseLayer::create(editorLayer);
            fakePauseLayer->saveLevel();
        }
        auto res = gmd::exportLevelAsGmd(EditorIDs::getLevelByID(levelId), filePath);

        std::ifstream lvlIn(filePath);
        std::ostringstream levelStr;
        levelStr << lvlIn.rdbuf();
        lvlIn.close();

        std::filesystem::remove(filePath);

        nm.send(
            SendLevelPacket::create(currentLobbyCode, swapIdx, levelStr.str())
        );
    }, nspace);
    nm.on<RecieveSwappedLevelPacket>([this](RecieveSwappedLevelPacket* packet) {
        auto gmdStr = std::string(packet->levels[swapIdx]);
        log::info("{}", packet->levels[swapIdx]);

        auto filePath = std::filesystem::temp_directory_path() / fmt::format("temp{}.gmd", rand());
        std::ofstream ostream(filePath);
        ostream << gmdStr;
        ostream.close();

        auto lvl = gmd::importGmdAsLevel(filePath);
        if (lvl) {
            LocalLevelManager::get()->m_localLevels->insertObject(*lvl, 0);

            auto scene = EditLevelLayer::scene(*lvl);
            cr::utils::replaceScene(scene);
        }

        std::filesystem::remove(filePath);
    }, nspace);
}
