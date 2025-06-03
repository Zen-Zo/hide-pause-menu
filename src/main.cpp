#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

constexpr int HIDDEN_OPACITY = 0;
constexpr int PAUSE_LAYER_OPACITY = 75;
constexpr int MAX_OPACITY = 255;
constexpr float BUTTON_SPRITE_SCALE = 0.67f;
constexpr float DEFAULT_BUTTON_SCALE = 1.0f;
constexpr float HIDDEN_BUTTON_SCALE = 35.f;

class $modify(HidePauseLayer, PauseLayer) {
	struct Fields {
		Mod* m_mod = Mod::get();
		CCNode* m_buttonPos;
		CCMenuItemSpriteExtra* m_button;
		std::unordered_set<std::string> m_nonVisibleChildren;
	};

	void customSetup() {
		PauseLayer::customSetup();
		if (!m_fields->m_mod->getSettingValue<bool>("hide_button_mod")) return;

		updateButtonPos();
		if (!m_fields->m_buttonPos) return;
	
		auto hideBtnSpr = CircleButtonSprite::create(CCSprite::createWithSpriteFrameName("hideBtn_001.png"));
		hideBtnSpr->setScale(BUTTON_SPRITE_SCALE);

		auto hideBtnButton = CCMenuItemSpriteExtra::create(
			hideBtnSpr, this, menu_selector(HidePauseLayer::onHideButton)
		);
		hideBtnButton->setID("hide-button"_spr);
		
		m_fields->m_buttonPos->addChild(hideBtnButton);
		m_fields->m_button = hideBtnButton;
	}

	void updateButtonPos() {
		std::string buttonPos = m_fields->m_mod->getSettingValue<std::string>("hide_button_pos") + "-button-menu";
		buttonPos[0] = std::tolower(buttonPos[0]);
		m_fields->m_buttonPos = getChildByID(buttonPos);
	}

	void onHideButton(CCObject* pSender) {
		const auto background = getChildByID("background");
		if (!background) return;

		const bool shouldShowUI = !background->isVisible();
		const bool screenshotMode = m_fields->m_mod->getSettingValue<bool>("screenshot_mode");

		changeChildrenVisibility(
			PauseLayer::getChildren(), 
			shouldShowUI, 
			m_fields->m_buttonPos->getID()
		);
		
		changeChildrenVisibility(
			m_fields->m_buttonPos->getChildren(), 
			shouldShowUI, 
			"hide-button"_spr
		);
		
		// Alphalaneous's Pages API and Vanilla Pages specific fix for overlapping buttons.
		// The visibility of the "{}-navigation-menu" nodes cant be changed for some reason.
		// If this is the case in the future, the next 4 lines can be removed.
		const auto rightNavigationMenu = getChildByID("right-button-menu-navigation-menu");
		if (rightNavigationMenu) changeChildrenVisibility(rightNavigationMenu->getChildren(), shouldShowUI);
		const auto centerNavigationMenu = getChildByID("center-button-menu-navigation-menu");
		if (centerNavigationMenu) changeChildrenVisibility(centerNavigationMenu->getChildren(), shouldShowUI);

		PauseLayer::setOpacity(shouldShowUI ? PAUSE_LAYER_OPACITY : HIDDEN_OPACITY);

		if (screenshotMode) updateButtonAppearance(shouldShowUI);
	}

	void changeChildrenVisibility(CCArray *children, const bool visibility, const std::string exceptionID = "") {
		for (CCNode* child : CCArrayExt<CCNode>(children)) {
			const std::string childID = child->getID();

			if (childID == exceptionID || m_fields->m_nonVisibleChildren.count(childID) > 0) continue;

			if (child->isVisible() != visibility) {
				child->setVisible(visibility);
			} else {
				m_fields->m_nonVisibleChildren.insert(childID);
			}
		}
	}

	void updateButtonAppearance(bool shouldShowUI) {
		if (!m_fields->m_button) return;

		if (shouldShowUI) {
			m_fields->m_button->setOpacity(MAX_OPACITY);
			m_fields->m_button->setScale(DEFAULT_BUTTON_SCALE);
		} else {
			m_fields->m_button->setOpacity(HIDDEN_OPACITY);
			m_fields->m_button->setScale(HIDDEN_BUTTON_SCALE); // Scale to the entire screen
		}
	}
};
