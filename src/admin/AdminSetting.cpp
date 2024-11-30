// this entire file was stolen from https://docs.geode-sdk.org/mods/settings#custom-settings

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

#include <network/manager.hpp>

#include "layers/AuthPopup.hpp"

using namespace geode::prelude;

class AdminButtonSetting : public SettingV3 {
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
        auto res = std::make_shared<AdminButtonSetting>();
        auto root = checkJson(json, "AdminButtonSetting");

        res->init(key, modID, root);
        res->parseNameAndDescription(root);
        res->parseEnableIf(root);
        
        root.checkUnknownKeys();
        return root.ok(std::static_pointer_cast<SettingV3>(res));
    }

    bool load(matjson::Value const& json) override {
        return true;
    }
    bool save(matjson::Value& json) const override {
        return true;
    }

    bool isDefaultValue() const override {
        return true;
    }
    void reset() override {}

    SettingNodeV3* createNode(float width) override;
};

class AdminButtonSettingNode : public SettingNodeV3 {
protected:
    ButtonSprite* m_buttonSprite;
    CCMenuItemSpriteExtra* m_button;

    bool init(std::shared_ptr<AdminButtonSetting> setting, float width) {
        if (!SettingNodeV3::init(setting, width))
            return false;
        
        m_buttonSprite = ButtonSprite::create("Admin Panel", "goldFont.fnt", "GJ_button_01.png", .8f);
        m_buttonSprite->setScale(.5f);
        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite, this, menu_selector(AdminButtonSettingNode::onButton)
        );
        this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
        this->getButtonMenu()->setContentWidth(60);
        this->getButtonMenu()->updateLayout();

        this->updateState(nullptr);
        
        return true;
    }
    
    void updateState(CCNode* invoker) override {
        SettingNodeV3::updateState(invoker);
        auto shouldEnable = this->getSetting()->shouldEnable();
        m_button->setEnabled(shouldEnable);
        m_buttonSprite->setCascadeColorEnabled(true);
        m_buttonSprite->setCascadeOpacityEnabled(true);
        m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
        m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
    }

    void onButton(CCObject*) {
        auto& nm = NetworkManager::get();
        nm.connect(true, []() {
            AuthPopup::create()->show();
        });
    }

    void onCommit() override {}
    void onResetToDefault() override {}

public:
    static AdminButtonSettingNode* create(std::shared_ptr<AdminButtonSetting> setting, float width) {
        auto ret = new AdminButtonSettingNode();
        if (ret && ret->init(setting, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool hasUncommittedChanges() const override {
        return false;
    }
    bool hasNonDefaultValue() const override {
        return false;
    }

    std::shared_ptr<AdminButtonSetting> getSetting() const {
        return std::static_pointer_cast<AdminButtonSetting>(SettingNodeV3::getSetting());
    }
};

// Create node as before
SettingNodeV3* AdminButtonSetting::createNode(float width) {
    return AdminButtonSettingNode::create(
        std::static_pointer_cast<AdminButtonSetting>(shared_from_this()),
        width
    );
}

// Register as before
$on_mod(Loaded) {
    (void)Mod::get()->registerCustomSettingType("admin-button", &AdminButtonSetting::parse);
}
