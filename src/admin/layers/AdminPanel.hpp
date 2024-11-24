#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdminPanel : public geode::Popup<> {
protected:
    bool setup() override;
public:
    static AdminPanel* create();
};

