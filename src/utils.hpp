#pragma once
#include <Geode/Geode.hpp>
#include <types/lobby.hpp>

using namespace geode::prelude;

namespace cr::utils {
    void goToScene(CCScene* scene);
    void replaceScene(CCScene* scene);
    void popScene();
    Account createAccountType();
};
