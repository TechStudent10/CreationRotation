#include "StatusPopup.hpp"

StatusPopup* StatusPopup::create(ZStringView title) {
    auto ret = new StatusPopup;
    if (ret->init(title)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool StatusPopup::init(ZStringView title) {
    if (!Popup::init(100.f, 75.f)) {
        return false;
    }

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

void StatusPopup::setStatus(ZStringView status) {
    statusLabel->setString(status.c_str());
}
