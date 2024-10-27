#include "AdminPanel.hpp"

AdminPanel* AdminPanel::create() {
    auto ret = new AdminPanel;
    if (ret->initAnchored(380.f, 265.f)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AdminPanel::setup() {
    this->setTitle("Administrator Panel");


    return true;
}
