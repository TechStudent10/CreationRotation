#include <Geode/Geode.hpp>
#include "SwapManager.hpp"

#include <network/manager.hpp>

#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include <matjson/reflect.hpp>

#include <layers/ChatPanel.hpp>
#include <utils.hpp>

using namespace geode::prelude;

#define CR_REQUIRE_CONNECTION() if(!NetworkManager::get().isConnected) return;

SwapManager::SwapManager() {
    auto& nm = NetworkManager::get();
    nm.onDisconnect([this](std::string) {
        this->currentLobbyCode = "";
    });
}

LobbySettings SwapManager::createDefaultSettings() {
    auto acc = cr::utils::createAccountType();

    return {
        .name = fmt::format("{}'s Lobby", acc.name),
        .turns = 4,
        .minutesPerTurn = 5,
        .owner = acc,
        .isPublic = false
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

    nm.send(JoinLobbyPacket::create(code, cr::utils::createAccountType()));

    nm.on<JoinedLobbyPacket>([this, callback, code](auto) {
        this->currentLobbyCode = code;
        callback();
    }, true);
}

void SwapManager::getLobbyAccounts(std::function<void(std::vector<Account>)> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();
    nm.send(GetAccountsPacket::create(this->currentLobbyCode));
    nm.on<ReceiveAccountsPacket>([this, callback](ReceiveAccountsPacket* accounts) {
        callback(accounts->accounts);
    });
}

void SwapManager::getLobbyInfo(std::function<void(LobbyInfo)> callback) {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();
    nm.send(GetLobbyInfoPacket::create(this->currentLobbyCode));
    nm.on<ReceiveLobbyInfoPacket>([this, callback](ReceiveLobbyInfoPacket* packet) {
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
    this->currentLobbyCode = "";
    ChatPanel::clearMessages();

    // no longer needed
    // nm.send(DisconnectFromLobbyPacket::create());
    nm.disconnect();
}

// LEVEL SWAP //

void SwapManager::startSwap(SwapStartedPacket* packet) {
    getLobbyInfo([this](LobbyInfo info) {
        secondsPerRound = info.settings.minutesPerTurn * 60;
    });

    auto glm = GameLevelManager::sharedState();
    auto newLvl = glm->createNewLevel();

    levelId = EditorIDs::getID(newLvl);

    registerListeners();

    roundStartedTime = time(0);

    auto scene = EditLevelLayer::scene(newLvl);
    cr::utils::replaceScene(scene);
}

void SwapManager::registerListeners() {
    CR_REQUIRE_CONNECTION()

    auto& nm = NetworkManager::get();

    nm.setDisconnectCallback([](std::string reason) {});

    nm.on<TimeToSwapPacket>([this](TimeToSwapPacket* p) {
        auto& nm = NetworkManager::get();

        Notification::create("Swapping levels!", NotificationIcon::Info, 2.5f)->show();

        auto filePath = std::filesystem::temp_directory_path() / fmt::format("temp{}.gmd", rand());

        if (auto editorLayer = LevelEditorLayer::get()) {
            auto fakePauseLayer = EditorPauseLayer::create(editorLayer);
            fakePauseLayer->saveLevel();
        }
        auto lvl = EditorIDs::getLevelByID(levelId);
        // this crashes macos when recieving the level
        // do not ask me why
        // this game is taped-together jerry-rigged piece of software
        // lvl->m_levelDesc = fmt::format("from: {}", this->createAccountType().name);

        LevelData lvlData = {
            .levelName = lvl->m_levelName,
            .songID = lvl->m_songID,
            .songIDs = lvl->m_songIDs,
            .levelString = lvl->m_levelString
        };

        nm.send(
            SendLevelPacket::create(lvlData)
        );

        if (lvl && Mod::get()->getSettingValue<bool>("delete-lvls")) {
            // let's not spam everyone's created levels list
            GameLevelManager::sharedState()->deleteLevel(lvl);
        }
    });
    nm.on<ReceiveSwappedLevelPacket>([this](ReceiveSwappedLevelPacket* packet) {
        // if (packet->levels.size() < swapIdx) {
        //     FLAlertLayer::create(
        //         "Creation Rotation",
        //         "<cr>There was an error while fetching the swapped level. If you're reading this, something has gone terribly wrong, please report it at once.</c>",
        //         "OK"
        //     )->show();
        //     return;
        // }
        LevelData lvlData;

        for (auto swappedLevel : packet->levels) {
            if (swappedLevel.accountID != cr::utils::createAccountType().accountID) continue;

            lvlData = swappedLevel.level;
            break;
        }

        auto lvl = GJGameLevel::create();

        lvl->m_levelName = lvlData.levelName;
        lvl->m_levelString = lvlData.levelString;
        lvl->m_songID = lvlData.songID;
        lvl->m_songIDs = lvlData.songIDs;

        lvl->m_levelType = GJLevelType::Editor;
        
        levelId = EditorIDs::getID(lvl);
        LocalLevelManager::get()->m_localLevels->insertObject(lvl, 0);

        #ifdef GEODE_IS_MACOS
        try {
        #endif
        auto scene = EditLevelLayer::scene(lvl);
        cr::utils::replaceScene(scene);
        #ifdef GEODE_IS_MACOS
        } catch (std::exception e) {}
        #endif

        roundStartedTime = time(0);
    });
    nm.on<SwapEndedPacket>([this](SwapEndedPacket* p) {
        log::debug("swap ended; disconnecting from server");
        auto& nm = NetworkManager::get();
        nm.showDisconnectPopup = false;
        this->disconnectLobby();
        FLAlertLayer::create(
            "Creation Rotation",
            "The Creation Rotation level swap has <cy>ended!</c>\nHope you had fun! :D\n\n<cl>You have been disconnected from the Creation Rotation server.</c>",
            "OK"
        )->show();
    });
}

int SwapManager::getTimeRemaining() {
    if (secondsPerRound <= 0) {
        return 0;
    }

    return (roundStartedTime + secondsPerRound) - time(0);
}
