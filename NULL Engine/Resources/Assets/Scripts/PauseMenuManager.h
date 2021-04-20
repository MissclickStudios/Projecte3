#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API PauseMenuManager : public Script {
public:
	PauseMenuManager();
	~PauseMenuManager();

	void Start() override;

	void Update() override;

	std::string buttonName = "continue";
	std::string buttonName1 = "options";
	std::string buttonName2 = "abandon run";
	std::string buttonName3 = "main menu";
	std::string buttonName4 = "exit";
private:
	C_UI_Button* continueButton;
	C_UI_Button* optionsButton;
	C_UI_Button* abandonRunButton;
	C_UI_Button* mainMenuButton;
	C_UI_Button* exitButton;
};

SCRIPTS_FUNCTION PauseMenuManager* CreatePauseMenuManager() {
	PauseMenuManager* script = new PauseMenuManager();
	INSPECTOR_STRING(script->buttonName);
	INSPECTOR_STRING(script->buttonName1);
	INSPECTOR_STRING(script->buttonName2);
	INSPECTOR_STRING(script->buttonName3);
	INSPECTOR_STRING(script->buttonName4);
	return script;
}