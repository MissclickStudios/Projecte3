#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_Canvas;
class C_UI_Button;
class C_UI_Chekbox;

class SCRIPTS_API MainMenuManager : public Script {
public:
	MainMenuManager();
	~MainMenuManager();
	//void Awake() override;
	void Start() override;
	//void PreUpdate()override;
	void Update() override;
	//void PostUpdate()override;
	//void CleanUp()override;
	GameObject* gameManager = nullptr;

	GameObject* mainCanvasObject = nullptr;
	GameObject* playButtonObject = nullptr;
	GameObject* continueButtonObject = nullptr;
	GameObject* exitButtonObject = nullptr;
	GameObject* settingsButtonObject = nullptr;

	GameObject* settingsCanvasObject = nullptr;
	GameObject* fullScreenCheckObject = nullptr;
	GameObject* vsyncCheckObject = nullptr;
	GameObject* backButtonObject = nullptr;
private:
	C_Canvas* mainCanvas = nullptr;
	C_UI_Button* playButton = nullptr;
	C_UI_Button* continueButton = nullptr;
	C_UI_Button* settingsButton = nullptr;
	C_UI_Button* exitButton = nullptr;

	C_Canvas* settingsCanvas = nullptr;
	C_UI_Checkbox* fullScreenCheck = nullptr;
	C_UI_Checkbox* vsyncCheck = nullptr;
	C_UI_Button* backButton = nullptr;
};

SCRIPTS_FUNCTION MainMenuManager* CreateMainMenuManager() {
	MainMenuManager* script = new MainMenuManager();
	INSPECTOR_GAMEOBJECT(script->gameManager);
	INSPECTOR_GAMEOBJECT(script->mainCanvasObject);
	INSPECTOR_GAMEOBJECT(script->playButtonObject);
	INSPECTOR_GAMEOBJECT(script->continueButtonObject);
	INSPECTOR_GAMEOBJECT(script->settingsButtonObject);
	INSPECTOR_GAMEOBJECT(script->exitButtonObject);
	INSPECTOR_GAMEOBJECT(script->settingsCanvasObject);
	INSPECTOR_GAMEOBJECT(script->vsyncCheckObject);
	INSPECTOR_GAMEOBJECT(script->mainCanvasObject);
	INSPECTOR_GAMEOBJECT(script->backButtonObject);
	return script;
}