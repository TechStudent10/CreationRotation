#include <Geode/Geode.hpp>

#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include <layers/Lobby.hpp>
#include <layers/LobbySelectPopup.hpp>
#include <layers/ChatPanel.hpp>

#include <network/manager.hpp>
#include <managers/SwapManager.hpp>
#include <managers/AuthManager.hpp>

#include <utils.hpp>

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
			if (auto lvlsBtn = menu->getChildByID("my-levels-button")) {
				if (auto lvlsBtnSpr = lvlsBtn->getChildByType<CCSprite>(0)) {
					cr::utils::scaleToMatch(lvlsBtnSpr, btnSpr, true);
				}
			}
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
		if (Mod::get()->getSettingValue<std::string>("server-url").find("prevter.me") != std::string::npos) {
			if (!Mod::get()->setSavedValue("seen-prevter-notice", true)) {
				Mod::get()->setSettingValue<std::string>("server-url", "wss://creationrotation.underscored.me");

				FLAlertLayer::create(
					"Creation Rotation",
					"We recently moved servers from <cy>prevter.me</c> to <cy>underscored.me</c>. The old server will not work, and as such the setting value has changed accordingly.",
					"OK"
				)->show();

				return;
			}
		}

		if (cr::utils::createAccountType().accountID == 0) {
			FLAlertLayer::create(
				"Creation Rotation",
				"Creation Rotation requires you to be logged in for verification purposes. Please sign in to a GD account.",
				"OK"
			)->show();
			return;
		}

		auto& sm = SwapManager::get();
		if (sm.currentLobbyCode == "") {
			auto& nm = NetworkManager::get();
			nm.connect(true, []() {
				auto& am = AuthManager::get();
				am.login([]() {
					LobbySelectPopup::create()->show();
				});
			});
		} else {
			auto layer = LobbyLayer::create(sm.currentLobbyCode);
			auto scene = CCScene::create();
			scene->addChild(layer);
			cr::utils::goToScene(scene);
		}
	}
};

class $modify(CREditorUI, EditorUI) {
	struct Fields {
		CCLabelBMFont* timerLabel;
	};

	bool init(LevelEditorLayer* editorLayer) {
		if (!EditorUI::init(editorLayer)) return false;

		auto& sm = SwapManager::get();
		if (sm.currentLobbyCode == "") return true;

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

		if (sm.getTimeRemaining() <= 0) {
			return;
		}

		auto timeDur = std::chrono::duration<int>(sm.getTimeRemaining());
		auto timeString = fmt::format("{:%M:%S}", timeDur);

		m_fields->timerLabel->setString(timeString.c_str());
	}
};

class $modify(CRLvlInfoLayer, LevelInfoLayer) {
	struct Fields {
		bool forceStartPlay = false;
	};

	void onPlay(CCObject* sender) {
		auto& sm = SwapManager::get();
		if (sm.currentLobbyCode == "" || m_fields->forceStartPlay) {
			LevelInfoLayer::onPlay(sender);
			return;
		}

		geode::createQuickPopup(
			"Creation Rotation",
			"You are currenly in a <cb>Creation Rotation</c> lobby. If you choose to play, note that you may be <cr>kicked out</c> of the game and into the level screen <cr>without saving</c> if the swap occurs. Are you sure you want to continue?",
			"No", "Play",
			[this, sender](auto, bool btn2) {
				if (!btn2) {
					return;
				}

				m_fields->forceStartPlay = true;
				this->onPlay(sender);
			}
		);
	}
};

// guidelines-menu
// help-button

class $modify(EditorPauseLayer) {
	bool init(LevelEditorLayer* lel) {
		if (!EditorPauseLayer::init(lel)) return false;

		auto& sm = SwapManager::get();
		if (sm.currentLobbyCode == "") {
			return true;
		}

		if (auto menu = this->getChildByID("guidelines-menu")) {
			auto msgButtonSpr = CCSprite::create("messagesBtn.png"_spr);
			if (auto helpBtn = menu->getChildByID("help-button")) {
				if (auto helpSpr = helpBtn->getChildByType<CCSprite>(0)) {
					cr::utils::scaleToMatch(helpSpr, msgButtonSpr, true);
				}
			}
			auto msgButton = CCMenuItemExt::createSpriteExtra(
				msgButtonSpr,
				[](CCObject*) {
					ChatPanel::create()->show();
				}
			);
			menu->addChild(msgButton);
			menu->updateLayout();
		}

		return true;
	}
};
