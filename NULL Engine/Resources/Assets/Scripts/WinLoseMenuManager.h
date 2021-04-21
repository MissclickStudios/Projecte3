#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API WinLoseMenuManager : public Script {
public:
	WinLoseMenuManager();
	~WinLoseMenuManager();
	void Start() override;
	void Update() override;

	std::string buttonName = "Hi";

private:
	C_UI_Button* backButton;
};

SCRIPTS_FUNCTION WinLoseMenuManager* CreateWinLoseMenuManager() {
	WinLoseMenuManager* script = new WinLoseMenuManager();
	INSPECTOR_STRING(script->buttonName);
	return script;
}