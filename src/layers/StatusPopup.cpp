#include "StatusPopup.hpp"

StatusPopup* StatusPopup::create(std::string title) {
    auto ret = new StatusPopup;
    if (ret->initAnchored(100.f, 75.f, title)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StatusPopup::setup(std::string title) {
    this->setTitle(title);

    statusLabel = CCLabelBMFont::create(
        "Loading...",
        "bigFont.fnt"
    );
    statusLabel->setScale(0.5f);

    this->addChildAtPosition(statusLabel, Anchor::Center);
    m_closeBtn->setVisible(false);

    return true;
}

void StatusPopup::setStatus(std::string status) {
    statusLabel->setString(status.c_str());
}
