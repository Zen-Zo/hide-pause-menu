#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

constexpr int HIDDEN_OPACITY = 0;
constexpr int PAUSE_LAYER_OPACITY = 75;
constexpr int MAX_OPACITY = 255;
constexpr float BUTTON_SPRITE_SCALE = 0.67f;
constexpr float DEFAULT_BUTTON_SCALE = 1.0f;
constexpr float HIDDEN_BUTTON_SCALE = 41.f; // Scale big enough to cover the entire screen
constexpr const char* NONE_EXECPTION_ID = "__NONE__";
constexpr const char* SHOW_PERCENTAGE = "0040";
constexpr const char* HIDE_PRACTICE_BUTTONS = "0071";
constexpr const char* SHOW_INFO_LABEL = "0109";
constexpr const char* HIDE_ATTEMPS = "0134";
constexpr const char* HIDE_PRACTICE_ATTEMPS = "0135";
constexpr const char* AUDIO_VISUALIZER = "0144";
constexpr const char* SHOW_TIME = "0145";

class $modify(HidePauseMenu, PauseLayer) {
	struct Fields {
		Mod* m_mod = Mod::get();
		GameManager* m_gameManager = GameManager::sharedState();
		PlayLayer* m_playLayer = PlayLayer::get();
		CCScene* m_scene = CCScene::get();
		CCNode* m_buttonPos;
		CCMenuItemSpriteExtra* m_button;
		std::unordered_set<CCNode*> m_nonVisibleNodes;
	};

	void customSetup() {
		PauseLayer::customSetup();
		if (!m_fields->m_mod->getSettingValue<bool>("hide_button_mod")) return;

		updateButtonPos();
		if (!m_fields->m_buttonPos) return;

		auto* hideBtnSpr = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("hideBtn_001.png"));
		hideBtnSpr->setScale(BUTTON_SPRITE_SCALE);

		auto* hideBtnButton = CCMenuItemSpriteExtra::create(hideBtnSpr, this, menu_selector(HidePauseMenu::onHideButton));
		hideBtnButton->setID("hide-button"_spr);
		
		m_fields->m_buttonPos->addChild(hideBtnButton);
		m_fields->m_buttonPos->updateLayout();
		m_fields->m_button = hideBtnButton;
	}

	void onResume(CCObject* sender) {
		if (!getChildByID("background")->isVisible())
			onHideButton(sender);

		PauseLayer::onResume(sender);
	}
	
	void tryQuit(CCObject* sender) {
		if (!getChildByID("background")->isVisible())
			onHideButton(sender);
		else
			PauseLayer::tryQuit(sender);
	}

	void onHideButton(CCObject* sender) {
		auto* background = getChildByID("background");
		if (!background) return;

		const bool shouldShowUI = !background->isVisible();

		changeChildrenVisibility(PauseLayer::getChildren(), shouldShowUI, m_fields->m_buttonPos->getID());
		changeChildrenVisibility(m_fields->m_buttonPos->getChildren(), shouldShowUI, "hide-button"_spr);
		
		PauseLayer::setOpacity(shouldShowUI ? PAUSE_LAYER_OPACITY : HIDDEN_OPACITY);
		applyUserModSettings(shouldShowUI);
		applyModCompatibilityFixes(shouldShowUI);

		if (m_fields->m_mod->getSettingValue<bool>("screenshot_mode")) {
			m_fields->m_button->setOpacity(shouldShowUI ? MAX_OPACITY : HIDDEN_OPACITY);
			m_fields->m_button->setScale(shouldShowUI ? DEFAULT_BUTTON_SCALE : HIDDEN_BUTTON_SCALE);
		}
	}

	void updateButtonPos() {
		auto buttonPos = m_fields->m_mod->getSettingValue<std::string>("hide_button_pos") + "-button-menu";
		buttonPos[0] = std::tolower(buttonPos[0]);
		m_fields->m_buttonPos = getChildByID(buttonPos);
	}

	void changeChildrenVisibility(CCArray* nodes, bool visible, const geode::ZStringView& exceptionID = NONE_EXECPTION_ID) {
		for (auto* node : CCArrayExt<CCNode>(nodes)) {
			if (node->getID() == exceptionID || m_fields->m_nonVisibleNodes.count(node) > 0) continue;
			
			if (node->isVisible() != visible)
				node->setVisible(visible);
			else
				m_fields->m_nonVisibleNodes.insert(node);
		}
	}
	
	void toggleNodeVisibility(CCNode* node, bool visible, bool condition) {
		if (node)
			node->setVisible(visible && condition);
	}

	void handlePlayLayerElements(PlayLayer* playLayer, Mod* mod, bool visible) {
		if (!playLayer) return;
		
		// Checkpoints
		if (playLayer->m_isPracticeMode && mod->getSettingValue<bool>("hide_checkpoints")) {
			for (CheckpointObject* checkpoint : CCArrayExt<CheckpointObject>(playLayer->m_checkpointArray)) {
				checkpoint->m_physicalCheckpointObject->setVisible(visible);
			}
		}

		handleUILayer(playLayer, m_fields->m_gameManager, mod, visible);
		handleGameManager(playLayer, m_fields->m_gameManager, mod, visible);
	}

	void handleUILayer(PlayLayer* playLayer, GameManager* gameManager, Mod* mod, bool visible) {
		if (!playLayer->m_uiLayer) return;

		if (mod->getSettingValue<bool>("hide_ui_layer")) {
			// UILayer
			playLayer->m_uiLayer->setVisible(visible);
		} else if (gameManager) {
			// Practice buttons
			if (playLayer->m_isPracticeMode && mod->getSettingValue<bool>("hide_practice_buttons")) {
				toggleNodeVisibility(playLayer->m_uiLayer->getChildByID("checkpoint-menu"), visible, !m_fields->m_gameManager->getGameVariable(HIDE_PRACTICE_BUTTONS));
			}

			// Audio visualizer
			if (mod->getSettingValue<bool>("hide_audio_visualizer")) {
				toggleNodeVisibility(playLayer->m_audioVisualizerSFX, visible, gameManager->getGameVariable(AUDIO_VISUALIZER));
				toggleNodeVisibility(playLayer->m_audioVisualizerBG, visible, gameManager->getGameVariable(AUDIO_VISUALIZER));
			}
		}
	}

	void handleGameManager(PlayLayer* playLayer, GameManager* gameManager, Mod* mod, bool visible) {
		if (!gameManager) return;

		if (playLayer->m_isPlatformer) {
			// Platformer time
			if (mod->getSettingValue<bool>("hide_time"))
				toggleNodeVisibility(playLayer->m_percentageLabel, visible, gameManager->getGameVariable(SHOW_TIME));
		} else {
			// Progress bar
			if (mod->getSettingValue<bool>("hide_progress_bar"))
				toggleNodeVisibility(playLayer->m_progressBar, visible, gameManager->m_showProgressBar);
			
			// Percentage
			if (mod->getSettingValue<bool>("hide_percentage"))
				toggleNodeVisibility(playLayer->m_percentageLabel, visible, gameManager->getGameVariable(SHOW_PERCENTAGE));
		}

		if (playLayer->m_isPracticeMode) {
			// Practice attemps
			if (!gameManager->getGameVariable(HIDE_ATTEMPS) && mod->getSettingValue<bool>("hide_practice_attemps"))
				toggleNodeVisibility(playLayer->m_attemptLabel, visible, !gameManager->getGameVariable(HIDE_PRACTICE_ATTEMPS));
		} else {
			// Attemps
			if (mod->getSettingValue<bool>("hide_attemps"))
				toggleNodeVisibility(playLayer->m_attemptLabel, visible, !gameManager->getGameVariable(HIDE_ATTEMPS));
		}
		
		// Level info label
		if (mod->getSettingValue<bool>("hide_info_label"))
			toggleNodeVisibility(playLayer->m_infoLabel, visible, gameManager->getGameVariable(SHOW_INFO_LABEL));
	}

	void handleSceneElements(CCScene* scene, Mod* mod, bool visible) {
		// Floating mod menu buttons
		if(mod->getSettingValue<bool>("hide_mod_menus")) {
			if (!scene) return;
			// This should hide other mods that use the scene layer and run on a scheduler, like some mod menus.
			for (auto* node : CCArrayExt<CCNode>(scene->getChildren())) {
				const auto id = node->getID();
				if (id == "PlayLayer" || id == "PauseLayer") continue;
				changeChildrenVisibility(node->getChildren(), visible);
			}
		}
	}

	void applyUserModSettings(bool visible) {
		handlePlayLayerElements(m_fields->m_playLayer, m_fields->m_mod, visible);
		handleSceneElements(m_fields->m_scene, m_fields->m_mod, visible);
	}

	// "Vanilla Pages" fix for overlapping buttons.
	void applyModCompatibilityFixes(bool visible) {
		if (auto* rightNavigationMenu = getChildByID("right-button-menu-navigation-menu"))
			changeChildrenVisibility(rightNavigationMenu->getChildren(), visible);

		if (auto* centerNavigationMenu = getChildByID("center-button-menu-navigation-menu"))
			changeChildrenVisibility(centerNavigationMenu->getChildren(), visible);
	}
};

class $modify(EndLevelLayer) {
	struct Fields {
		Mod* m_mod = Mod::get();
		HidePauseMenu m_hpm = HidePauseMenu();
		CCMenuItemSpriteExtra* m_hideButton = nullptr;
	};

	void customSetup() {
		EndLevelLayer::customSetup();
		if (auto* hideLayerMenu = getChildByID("hide-layer-menu"))
			m_fields->m_hideButton = typeinfo_cast<CCMenuItemSpriteExtra*>(hideLayerMenu->getChildByID("hide-button"));
	}

	void onHideLayer(CCObject* sender) {
		EndLevelLayer::onHideLayer(sender);
		setLayerVisibility(!EndLevelLayer::m_hidden);
	}

	void onReplay(CCObject* sender) {
		if (EndLevelLayer::m_hidden)
			setLayerVisibility();

		EndLevelLayer::onReplay(sender);
	}

	void onMenu(CCObject* sender) {
		if (EndLevelLayer::m_hidden)
			onHideLayer(sender);
		else
			EndLevelLayer::onMenu(sender);
	}

	void setLayerVisibility(bool visible = true) {
		if (!m_fields->m_mod->getSettingValue<bool>("hide_button_mod")) return;

		if (m_fields->m_mod->getSettingValue<bool>("end_level_button"))
			m_fields->m_hpm.applyUserModSettings(visible);
		
		if (m_fields->m_mod->getSettingValue<bool>("end_level_screenshot_mode"))
			updateHideButton(visible);
	}

	void updateHideButton(bool visible) {
		if (!m_fields->m_hideButton) return;

		m_fields->m_hideButton->setOpacity(visible ? MAX_OPACITY : HIDDEN_OPACITY);
		m_fields->m_hideButton->setScale(visible ? DEFAULT_BUTTON_SCALE : HIDDEN_BUTTON_SCALE);
	}
};