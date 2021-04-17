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

	std::string buttonName = "Hi";
	std::string buttonName1 = "Hi1";
	std::string buttonName2 = "Hi2";
	std::string buttonName3 = "Hi3";
	std::string buttonName4 = "Hi4";
private:
	C_UI_Button* continueButton;
	C_UI_Button* optionsButton;
	C_UI_Button* abandonRunButton;
	C_UI_Button* mainMenuButton;
	C_UI_Button* exitButton;
};

SCRIPTS_FUNCTION PauseMenuManager* CreateMainMenuManager() {
	PauseMenuManager* script = new PauseMenuManager();
	INSPECTOR_STRING(script->buttonName);
	INSPECTOR_STRING(script->buttonName1);
	INSPECTOR_STRING(script->buttonName2);
	INSPECTOR_STRING(script->buttonName3);
	INSPECTOR_STRING(script->buttonName4);
	return script;
}