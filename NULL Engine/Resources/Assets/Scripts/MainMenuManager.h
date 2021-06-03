#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_Canvas;
class C_UI_Button;
class C_UI_Chekbox;
class C_UI_Slider;

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
	GameObject* musicSliderObject = nullptr;
	GameObject* fxSliderObject = nullptr;
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
	C_UI_Slider* musicSlider = nullptr;
	C_UI_Slider* fxSlider = nullptr;
	float volume = 0.0f;
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
	INSPECTOR_GAMEOBJECT(script->fullScreenCheckObject);
	INSPECTOR_GAMEOBJECT(script->vsyncCheckObject);
	INSPECTOR_GAMEOBJECT(script->mainCanvasObject);
	INSPECTOR_GAMEOBJECT(script->backButtonObject);
	INSPECTOR_GAMEOBJECT(script->musicSliderObject);
	INSPECTOR_GAMEOBJECT(script->fxSliderObject);
	return script;
}