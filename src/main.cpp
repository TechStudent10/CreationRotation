#include <Geode/Geode.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include <layers/LobbySelectPopup.hpp>

#include <network/manager.hpp>

using namespace geode::prelude;

class $modify(CRBrowserLayer, LevelBrowserLayer) {
	bool init(GJSearchObject* searchObject) {
		if (!LevelBrowserLayer::init(searchObject)) {
			return false;
		}

		if (searchObject->m_searchType != SearchType::MyLevels) return true;
		
		auto btnSpr = CCSprite::create("menu-btn.png"_spr);
		// the images aren't large anymore :D
		// btnSpr->setScale(0.1f); // these images are LARGE
		auto myButton = CCMenuItemSpriteExtra::create(
			btnSpr,
			this,
			menu_selector(CRBrowserLayer::onMyButton)
		);

		auto menu = this->getChildByID("my-levels-menu");
		menu->addChild(myButton);

		myButton->setID("rotation-start"_spr);

		menu->updateLayout();

		return true;
	}

	void onMyButton(CCObject*) {
		NetworkManager::get().connect();
		LobbySelectPopup::create()->show();
	}
};

