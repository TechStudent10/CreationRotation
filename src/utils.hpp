#pragma once
#include <Geode/Geode.hpp>
#include <types/lobby.hpp>

using namespace geode::prelude;

namespace cr::utils {
    void goToScene(CCScene* scene);
    void replaceScene(CCScene* scene);
    void popScene();

    void scaleToMatch(CCNode* orig, CCNode* node, bool isSquare = false);

    static Account createAccountType() {
        auto gm = GameManager::get();

        return {
            .name = gm->m_playerName,
            .userID = gm->m_playerUserID.value(),
            .accountID = GJAccountManager::sharedState()->m_accountID,
            .iconID = gm->getPlayerFrame(),
            .color1 = gm->m_playerColor.value(),
            .color2 = gm->m_playerColor2.value(),
            .color3 = gm->m_playerGlow ?
                gm->m_playerGlowColor.value() :
                -1
        };
    }
};
