#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

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
	GameObject* playButtonObject = nullptr;
	GameObject* continueButtonObject = nullptr;
	GameObject* exitButtonObject = nullptr;
	GameObject* gameManager = nullptr;
	GameObject* canvasObject = nullptr;
private:
	C_UI_Button* playButton = nullptr;
	C_UI_Button* continueButton = nullptr;
	C_UI_Button* exitButton = nullptr;
};

SCRIPTS_FUNCTION MainMenuManager* CreateMainMenuManager() {
	MainMenuManager* script = new MainMenuManager();
	INSPECTOR_GAMEOBJECT(script->playButtonObject);
	INSPECTOR_GAMEOBJECT(script->continueButtonObject);
	INSPECTOR_GAMEOBJECT(script->exitButtonObject);
	INSPECTOR_GAMEOBJECT(script->gameManager);
	INSPECTOR_GAMEOBJECT(script->canvasObject);
	return script;
}