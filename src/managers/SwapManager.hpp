#pragma once
#include <Geode/Geode.hpp>

#include <types/lobby.hpp>
#include <network/packets/client.hpp>
#include <network/packets/server.hpp>

#include <defs.hpp>

class CR_DLL SwapManager {
public:
    static SwapManager& get() {
        static SwapManager instance;
        return instance;
    }
    
    std::string currentLobbyCode;

    void joinLobby(std::string code, std::function<void()> callback = []() {});
    void createLobby(LobbySettings lobby, std::function<void(std::string)> callback);
    void updateLobby(LobbySettings updatedLobby);
    void disconnectLobby();

    void getLobbyAccounts(std::function<void(std::vector<Account>)>);
    void getLobbyInfo(std::function<void(LobbyInfo)>);

    static LobbySettings createDefaultSettings();

    // LEVEL SWAP //

    int secondsPerRound;

    int roundStartedTime;
    int roundEndingTime;

    int swapIdx;
    int levelId;

    void startSwap(SwapStartedPacket* packet);

    void registerListeners();
    int getTimeRemaining();

protected:
    SwapManager();
};