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
	};

	void customSetup() {
		PauseLayer::customSetup();
		if (!m_fields->m_mod->getSettingValue<bool>("hide_button_mod")) return;

		this->updateButtonPos();
		if (!m_fields->m_buttonPos) return; // Should never happen, unless another mod remove the "{}-button-menu" node.
	
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
		if (!background) return; // Should never happen, unless another mod remove the "background" node.

		const bool shouldShowUI = !background->isVisible();
		const bool screenshotMode = m_fields->m_mod->getSettingValue<bool>("screenshot_mode");

		this->changeChildsVisibility(
			PauseLayer::getChildren(), 
			shouldShowUI, 
			m_fields->m_buttonPos->getID()
		);

		this->changeChildsVisibility(
			m_fields->m_buttonPos->getChildren(), 
			shouldShowUI, 
			screenshotMode ? "" : "hide-button"_spr
		);

		PauseLayer::setOpacity(shouldShowUI ? PAUSE_LAYER_OPACITY : HIDDEN_OPACITY);

		if (screenshotMode) {
			if (shouldShowUI) {
				m_fields->m_button->setOpacity(MAX_OPACITY);
				m_fields->m_button->setScale(DEFAULT_BUTTON_SCALE);
			} else {
				m_fields->m_button->setOpacity(HIDDEN_OPACITY);
				// Setting opacity to 0 on a node sets the visibility to false, making it not clickable
				m_fields->m_button->setVisible(true);
				// Scale to the entire screen
				m_fields->m_button->setScale(HIDDEN_BUTTON_SCALE);
			}
		}
	}

	void changeChildsVisibility(CCArray *childs, const bool visible, const std::string exceptionID = "") {
		for (CCNode* child : CCArrayExt<CCNode>(childs)) {
			if (child->getID() != exceptionID)
				child->setVisible(visible);
		}
	}
};
