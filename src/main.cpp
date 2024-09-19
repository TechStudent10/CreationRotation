#include <Geode/Geode.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include <layers/LobbySelectPopup.hpp>

#include <network/manager.hpp>
#include <network/packets/server.hpp>
#include <network/packets/client.hpp>

#include <managers/SwapManager.hpp>

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
		LobbySelectPopup::create()->show();
	}
};

// class $modify(CREditorLayer, LevelEditorLayer) {
// 	bool init(GJGameLevel* level, bool smth) {
// 		if (!LevelEditorLayer::init(level, smth)) return false;

// 		auto& nm = NetworkManager::get();
// 		auto& lm = SwapManager::get();
// 		auto nspace = fmt::format("/{}", lm.currentLobbyCode);
// 		nm.on<TimeToSwapPacket>([this, nspace](TimeToSwapPacket* p) {
// 			auto& lm = SwapManager::get();
// 			auto& nm = NetworkManager::get();
// 			log::info("maybe?");
// 			nm.send(
// 				SendLevelPacket::create(lm.currentLobbyCode, lm.swapIdx, this->getLevelString())
// 			);
// 		}, nspace);
// 		nm.on<RecieveSwappedLevelPacket>([this, lm](RecieveSwappedLevelPacket* packet) {
// 			log::info("?????????");
// 			log::info("{}", packet->levels[lm.swapIdx]);
// 		}, nspace);

// 		return true;
// 	}
// };
