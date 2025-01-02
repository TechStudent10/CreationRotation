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

// scales up `node` based on the content size of `orig`
// 
// if `isSquare` is on, it averages out the scaleX and scaleY
void cr::utils::scaleToMatch(CCNode* orig, CCNode* node, bool isSquare) {
    float scaleFactorX = orig->getContentWidth() / node->getContentWidth();
    float scaleFactorY = orig->getContentHeight() / node->getContentHeight();

    if (isSquare) {
        node->setScale((scaleFactorX + scaleFactorY) / 2.f);
    } else {
        node->setScaleX(scaleFactorX);
        node->setScaleY(scaleFactorY);
    }
}
