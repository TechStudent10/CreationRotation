#include "utils.hpp"

void cr::utils::goToScene(CCScene *scene) {
    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
}

void cr::utils::replaceScene(CCScene *scene) {
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
}

void cr::utils::popScene() {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

Account cr::utils::createAccountType() {
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
