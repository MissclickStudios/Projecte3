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
	std::string buttonName = "play";
	std::string buttonName1 = "continue";
	std::string buttonName2 = "options";
	std::string buttonName3 = "exit";
private:
	C_UI_Button* playButton;
	C_UI_Button* continueButton;
	C_UI_Button* optionsButton;
	C_UI_Button* exitButton;
};

SCRIPTS_FUNCTION MainMenuManager* CreateMainMenuManager() {
	MainMenuManager* script = new MainMenuManager();
	INSPECTOR_STRING(script->buttonName);
	INSPECTOR_STRING(script->buttonName1);
	INSPECTOR_STRING(script->buttonName2);
	INSPECTOR_STRING(script->buttonName3);
	return script;
}