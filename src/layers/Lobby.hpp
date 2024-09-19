#include <Geode/Geode.hpp>
#include <types/lobby.hpp>

using namespace geode::prelude;

class PlayerCell : public CCLayer {
protected:
    bool init(std::string playerName, int mainColor, int secondColor, int glowColor, int iconID, float width);
    void onOpenProfile(CCObject*);
public:
    static constexpr int CELL_HEIGHT = 75.f;
    static PlayerCell* create(std::string playerName, int mainColor, int secondColor, int glowColor, int iconID, float width);
};

class LobbyLayer : public CCLayer {
protected:
    std::string lobbyCode;
    bool isOwner = false;

    std::string lobbyNspace;

    CCMenuItemSpriteExtra* closeBtn;
    CCSprite* background;

    CCArray* playerListItems;
    CustomListView* playerList;

    CCLabelBMFont* titleLabel;

    CCMenuItemSpriteExtra* settingsBtn;
    CCMenuItemSpriteExtra* startBtn;

    LoadingCircle* loadingCircle;
    CCLayer* mainLayer;

    bool init(std::string code);
    void keyBackClicked();
    void createBorders();

    void refresh(LobbyInfo info);

    void registerListeners();
    void unregisterListeners();

    void onClose(CCObject*);
    void onStart(CCObject*);
    void onSettings(CCObject*);

    virtual ~LobbyLayer();
public:
    static LobbyLayer* create(std::string code);
};

