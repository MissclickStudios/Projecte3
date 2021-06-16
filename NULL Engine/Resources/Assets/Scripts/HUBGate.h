#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class GameManager;
class C_Canvas;
class C_UI_Button;
class Player;

class SCRIPTS_API HUBGate : public Script {
public:
	HUBGate();
	~HUBGate();

	void Start()override;
	void Update() override;

	void OnCollisionEnter(GameObject* object) override;
	GameObject* gameManagerObject = nullptr;
	std::string playerStr = "Mandalorian";
	GameObject* popUpCanvasObject = nullptr;
	GameObject* yesButtonObject = nullptr;
	//GameObject* noButtonObject = nullptr;

private:
	GameManager* gameManager = nullptr;
	Player* player = nullptr;
	C_Canvas* popUpCanvas = nullptr;
	C_UI_Button* yesButton = nullptr;
	//C_UI_Button* noButton = nullptr;

	bool triggered = false;
};

SCRIPTS_FUNCTION HUBGate* CreateHUBGate();