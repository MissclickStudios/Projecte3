#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class Player;
class C_Animator;
class C_Canvas;
class C_UI_Button;
class C_UI_Slider;
class C_UI_Text;
class GameManager;

class ItemData;

enum class HUBArmorerState: char {
	ACTIVE,
	INACTIVE,
	NONE
};

class SCRIPTS_API HUBArmorer : public Script {
public:
	HUBArmorer();
	~HUBArmorer();
	void Start() override;
	void Update() override;

	void UpdateMenu();

	std::string mandoName = "Mandalorian"; 
	std::string hubShopCanvasName = "ArmorerShopCanvas";

	float talkDistance = 10.f;

	bool menuOpen = false;

	GameObject* armorButtonObject = nullptr;
	GameObject* bootsButtonObject = nullptr;
	GameObject* ticketButtonObject = nullptr;
	GameObject* bottleButtonObject = nullptr;
	GameObject* armorSliderObject = nullptr;
	GameObject* ticketSliderObject = nullptr;
	GameObject* bootsSliderObject = nullptr;
	GameObject* bottleSliderObject = nullptr;
	GameObject* armorTextObject = nullptr;
	GameObject* bootsTextObject = nullptr;
	GameObject* ticketTextObject = nullptr;
	GameObject* bottleTextObject = nullptr;
	GameObject* titleTextObject = nullptr;
	GameObject* descriptionTextObject = nullptr;
	GameObject* creditTextObject = nullptr;
	GameObject* beskarTextObject = nullptr;

private:
	C_Canvas* hubShopCanvas = nullptr;
	Player* mando = nullptr;
	GameManager* gameManager = nullptr;
	HUBArmorerState state = HUBArmorerState::NONE;

	C_Animator* armorerAnimator = nullptr;

	C_UI_Button* armorButton = nullptr;
	C_UI_Button* bootsButton = nullptr;
	C_UI_Button* ticketButton = nullptr;
	C_UI_Button* bottleButton = nullptr;
	C_UI_Slider* armorSlider = nullptr;
	C_UI_Slider* ticketSlider = nullptr;
	C_UI_Slider* bootsSlider = nullptr;
	C_UI_Slider* bottleSlider = nullptr;
	C_UI_Text* armorPriceText = nullptr;
	C_UI_Text* bootsPriceText = nullptr;
	C_UI_Text* ticketPriceText = nullptr;
	C_UI_Text* bottlePriceText = nullptr;
	C_UI_Text* titleText = nullptr;
	C_UI_Text* descriptionText = nullptr;
	C_UI_Text* creditsText = nullptr;
	C_UI_Text* beskarText = nullptr;

	std::vector<ItemData*> hubItems;
};

SCRIPTS_FUNCTION HUBArmorer* CreateHUBArmorer();