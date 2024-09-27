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
