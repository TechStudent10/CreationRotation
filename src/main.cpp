#include <Geode/Geode.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

#include <layers/LobbySelectPopup.hpp>

#include <network/manager.hpp>
#include <managers/SwapManager.hpp>

#include <fmt/chrono.h>

using namespace geode::prelude;

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

		auto menu = this->getChildByID("my-levels-menu");
		menu->addChild(myButton);

		myButton->setID("rotation-start"_spr);

		menu->updateLayout();

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
