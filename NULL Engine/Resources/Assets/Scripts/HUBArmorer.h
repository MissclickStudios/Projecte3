#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class Player;
class C_Canvas;
class C_UI_Button;
class C_UI_Slider;
class C_UI_Text;
class GameManager;

enum class HUBArmorerState: char {
	ACTIVE,
	INACTIVE,
};

class SCRIPTS_API HUBArmorer : public Script {
public:
	HUBArmorer();
	~HUBArmorer();
	void Start() override;
	void Update() override;

	std::string mandoName = "Mando testbuild"; 
	std::string hubShopCanvasName = "HUBShopCanvas";

	float talkDistance = 10.f;

	bool menuOpen = false;

	GameObject* armorButtonObject = nullptr;
	GameObject* bootsButtonObject = nullptr;
	GameObject* ticketButtonObject = nullptr;
	GameObject* bottleButtonObject = nullptr;
	GameObject* armorSliderObject = nullptr;
	GameObject* ticketSliderObject = nullptr;
	GameObject* creditsSliderObject = nullptr;
	GameObject* bottleSliderObject = nullptr;
	GameObject* armorTextObject = nullptr;
	GameObject* bootsTextObject = nullptr;
	GameObject* ticketTextObject = nullptr;
	GameObject* bottleTextObject = nullptr;
	GameObject* descriptionTextObject = nullptr;

private:
	C_Canvas* hubShopCanvas = nullptr;
	Player* mando = nullptr;
	GameManager* gameManager = nullptr;
	HUBArmorerState state = HUBArmorerState::INACTIVE;

	C_UI_Button* armorButton = nullptr;
	C_UI_Button* bootsButton = nullptr;
	C_UI_Button* ticketButton = nullptr;
	C_UI_Button* bottleButton = nullptr;
	C_UI_Slider* armorSlider = nullptr;
	C_UI_Slider* ticketSlider = nullptr;
	C_UI_Slider* creditsSlider = nullptr;
	C_UI_Slider* bottleSlider = nullptr;
	C_UI_Text* armortext = nullptr;
	C_UI_Text* bootstext = nullptr;
	C_UI_Text* tickettext = nullptr;
	C_UI_Text* bottletext = nullptr;
	C_UI_Text* descriptionText = nullptr;

};

SCRIPTS_FUNCTION HUBArmorer* CreateHUBArmorer();