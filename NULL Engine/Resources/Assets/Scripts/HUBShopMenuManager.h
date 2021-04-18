#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API HUBShopMenuManager : public Script {
public:
	HUBShopMenuManager();
	~HUBShopMenuManager();
	void Start() override;
	void Update() override;

	std::string buttonName = "Hi";

private:
	C_UI_Button* button;
};

SCRIPTS_FUNCTION HUBShopMenuManager* CreateHUBShopMenuManager() {
	HUBShopMenuManager* script = new HUBShopMenuManager();
	INSPECTOR_STRING(script->buttonName);
	return script;
}