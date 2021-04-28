#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;
class C_UI_Text;

class SCRIPTS_API HUBShopMenuManager : public Script {
public:
	HUBShopMenuManager();
	~HUBShopMenuManager();
	void Start() override;
	void Update() override;

	std::string exitButtonName = "ExitButton";
	std::string buyButtonName = "BuyButton";
	std::string mandoName = "Mandalorian";
	std::string beskarTextHubShopName = "BeskarTextHubShop";

	int beskarCost = 2;

private:
	C_UI_Button* exitButton = nullptr;
	C_UI_Button* buyButton = nullptr;

	C_UI_Text* beskarTextHubShop = nullptr;

	GameObject* mando = nullptr;
};

SCRIPTS_FUNCTION HUBShopMenuManager* CreateHUBShopMenuManager() {
	HUBShopMenuManager* script = new HUBShopMenuManager();

	INSPECTOR_STRING(script->beskarTextHubShopName);
	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->exitButtonName);
	INSPECTOR_STRING(script->buyButtonName);
	return script;
}