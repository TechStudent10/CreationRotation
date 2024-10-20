#include <Geode/Geode.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

#include <layers/LobbySelectPopup.hpp>

#include <network/manager.hpp>
#include <managers/SwapManager.hpp>

#include <fmt/chrono.h>

using namespace geode::prelude;

// ok so this is really weird but
// if you're on wine or something, the CertStore which im assuming contains certificates and whatnot
// can simply just not exist, and ixwebsocket needs it to. this code will create it if it doesnt exist
// credits to dankmeme01 for the fix (i literally just copy/pasted it from his message)
#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#include <wincrypt.h>

$on_mod(Loaded) {
	DWORD flags = CERT_STORE_READONLY_FLAG | CERT_SYSTEM_STORE_CURRENT_USER;
	HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, flags, L"Root");
	if (store) {
		CertCloseStore(store, flags);
	}
};
#endif

class $modify(CRBrowserLayer, LevelBrowserLayer) {
	bool init(GJSearchObject* searchObject) {
		if (!LevelBrowserLayer::init(searchObject)) {
			return false;
		}

		if (searchObject->m_searchType != SearchType::MyLevels) return true;
		
		auto btnSpr = CCSprite::create("menu-btn.png"_spr);
		// the images aren't large anymore :D
		btnSpr->setScale(0.275f); // these images are LARGE
		auto myButton = CCMenuItemSpriteExtra::create(
			btnSpr,
			this,
			menu_selector(CRBrowserLayer::onMyButton)
		);
		myButton->setZOrder(2);
		myButton->setID("rotation-start"_spr);

		auto menu = this->getChildByID("my-levels-menu");
		if (menu) {
			menu->addChild(myButton);
			menu->updateLayout();
		} else {
			auto menu = CCMenu::create();
			menu->addChild(myButton);
			menu->setID("cr-menu"_spr);
			menu->setPosition({
				30.75, 69.5
			});
			this->addChild(menu);
		}

		return true;
	}

	void onMyButton(CCObject*) {
		auto& nm = NetworkManager::get();
		nm.connect(true);
		LobbySelectPopup::create()->show();
	}
};

class $modify(CREditorUI, EditorUI) {
	struct Fields {
		CCLabelBMFont* timerLabel;
	};

	bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) return false;

		auto& nm = NetworkManager::get();
		if (!nm.isConnected) return true;

		m_fields->timerLabel = CCLabelBMFont::create(
			"00:00",
			"bigFont.fnt"
		);
		m_fields->timerLabel->setScale(0.5f);
		m_fields->timerLabel->setOpacity(0.5f * 255.f);
		
		auto sliderPos = m_positionSlider->m_sliderBar->convertToWorldSpace({0, 0});
		m_fields->timerLabel->setPosition({
			CCDirector::sharedDirector()->getWinSize().width / 2.f,
			sliderPos.y - 35.f
		});
		m_fields->timerLabel->setAnchorPoint({0.5f, 0.5f});

		this->schedule(schedule_selector(CREditorUI::updateTimer), 1);

		this->addChild(m_fields->timerLabel);

		return true;
	}

	void showUI(bool shouldShow) {
		EditorUI::showUI(shouldShow);

		if (!m_fields->timerLabel) return;

		m_fields->timerLabel->setVisible(shouldShow);
	}

	void updateTimer(float dt) {
		if (!m_fields->timerLabel) return;

		auto& sm = SwapManager::get();

		auto timeDur = std::chrono::duration<int>(sm.getTimeRemaining());
		auto timeString = fmt::format("{:%M:%S}", timeDur);

		m_fields->timerLabel->setString(timeString.c_str());
	}
};
