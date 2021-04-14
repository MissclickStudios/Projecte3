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
	std::string buttonName = "Hi";
private:
	C_UI_Button* playButton;
};

SCRIPTS_FUNCTION MainMenuManager* CreateMainMenuManager() {
	MainMenuManager* script = new MainMenuManager();
	INSPECTOR_STRING(script->buttonName);
	return script;
}