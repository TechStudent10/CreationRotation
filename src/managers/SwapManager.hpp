#pragma once
#include <Geode/Geode.hpp>
#include <sio_client.h>

#include <types/lobby.hpp>

class SwapManager {
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

    static Account createAccountType();
    static LobbySettings createDefaultSettings();

    // LEVEL SWAP //

    int swapIdx;
    int levelId;

    void registerListeners();

protected:
    SwapManager();
};