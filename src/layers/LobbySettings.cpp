#include "LobbySettings.hpp"

enum LobbySettingType {
    Name,
    Turns,
    MinsPerTurn,
    Password
};

class LobbySettingsCell : public CCNode {
protected:
    bool init(float width, std::string name, LobbySettingType type, std::string desc, std::string defaultStr = "", std::string filter = "") {
        this->type = type;
        
        this->setContentSize({
            width, CELL_HEIGHT
        });

        auto nameLabel = CCLabelBMFont::create(
            name.c_str(),
            "bigFont.fnt"
        );
        
        nameLabel->setPosition({
            5.f, CELL_HEIGHT / 2.f
        });
        nameLabel->setAnchorPoint({
            0.f, 0.5f
        });
        nameLabel->limitLabelWidth(80.f, 0.5f, 0.1f);
        this->addChild(nameLabel);

        auto infoBtn = InfoAlertButton::create(
            name,
            desc,
            0.5f
        );
        auto menu = CCMenu::create();
        menu->addChild(infoBtn);
        menu->setPosition({
            nameLabel->getScaledContentWidth() + 15.f, CELL_HEIGHT / 2.f
        });
        menu->setAnchorPoint({
            0.f, 0.5f
        });
        this->addChild(menu);

        input = TextInput::create(95.f, name);
        input->setString(defaultStr);
        if (filter != "") input->getInputNode()->setAllowedChars(filter);
        input->setPosition({
            width - input->getContentWidth() - 5.f, CELL_HEIGHT / 2.f
        });
        input->setAnchorPoint({
            0.f, 0.5f
        });
        this->addChild(input);

        return true;
    }
public:
    static constexpr int CELL_HEIGHT = 35.f;
    TextInput* input;
    LobbySettingType type;

    static LobbySettingsCell* create(float width, std::string name, LobbySettingType type, std::string desc, std::string defaultStr = "", std::string filter = "") {
        auto ret = new LobbySettingsCell;
        if (ret->init(width, name, type, desc, defaultStr, filter)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

LobbySettingsPopup* LobbySettingsPopup::create(LobbySettings settings, Callback callback) {
    auto ret = new LobbySettingsPopup;
    if (ret->initAnchored(250.f, 230.f, settings, callback)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LobbySettingsPopup::setup(LobbySettings settings, Callback callback) {
    m_noElasticity = true;
    this->setTitle("Lobby Settings");

    CCNode* nameContainer = CCNode::create();
    nameContainer->setLayout(
        RowLayout::create()
            ->setGap(-5.f)
    );

    auto settingsContents = CCArray::create();

    #define ADD_SETTING(name, element, desc, type) settingsContents->addObject( \
        LobbySettingsCell::create( \
            220.f, \
            name,\
            LobbySettingType::type, \
            desc, \
            settings.element \
        ));

    #define ADD_SETTING_INT(name, element, desc, type) settingsContents->addObject( \
        LobbySettingsCell::create( \
            220.f, \
            name,\
            LobbySettingType::type, \
            desc, \
            std::to_string(settings.element), \
            "0123456789" \
        ));

    ADD_SETTING("Name", name, "Name of the lobby", Name)
    ADD_SETTING_INT("Turns", turns, "Number of turns per level", Turns)
    ADD_SETTING_INT("Minutes per turn", minutesPerTurn, "Amount of minutes per turn", MinsPerTurn)
    // ADD_SETTING("Password", password, Password)

    auto settingsList = ListView::create(settingsContents, LobbySettingsCell::CELL_HEIGHT, 220.f, 180.f);
    settingsList->ignoreAnchorPointForPosition(false);

    auto border = Border::create(
        settingsList,
        {0, 0, 0, 75},
        {220.f, 180.f}
    );
    if(CCScale9Sprite* borderSprite = typeinfo_cast<CCScale9Sprite*>(border->getChildByID("geode.loader/border_sprite"))) {
        float scaleFactor = 1.7f;
        borderSprite->setContentSize(CCSize{borderSprite->getContentSize().width, borderSprite->getContentSize().height + 3} / scaleFactor);
        borderSprite->setScale(scaleFactor);
        borderSprite->setPositionY(-0.5);
    }
    border->ignoreAnchorPointForPosition(false);

    m_mainLayer->addChildAtPosition(border, Anchor::Center, ccp(0, -10.f));

    auto submitBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Submit"),
        [this, settingsContents, callback](CCObject* sender) {
            this->onClose(this->m_closeBtn);
            
            LobbySettings newSettings = SwapManager::createDefaultSettings();

            for (auto cell : CCArrayExt<LobbySettingsCell*>(settingsContents)) {
                switch (cell->type) {
                    case Name: {
                        newSettings.name = cell->input->getString();
                        break;
                    }
                    case Turns: {
                        newSettings.turns = geode::utils::numFromString<int>(
                            cell->input->getString()
                        ).unwrapOr(0);
                        break;
                    }
                    case MinsPerTurn: {
                        newSettings.minutesPerTurn = geode::utils::numFromString<int>(
                            cell->input->getString()
                        ).unwrapOr(1);
                        break;
                    }
                    case Password: {
                        // newSettings.password = cell->input->getString();
                        break;
                    }
                }
            }

            callback(newSettings);
        }
    );
    auto menu = CCMenu::create();
    menu->addChild(submitBtn);

    m_mainLayer->addChildAtPosition(menu, Anchor::Bottom);

    return true;
}
