#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AuthPopup : public geode::Popup {
protected:
    bool init() override;
public:
    static AuthPopup* create();
};
