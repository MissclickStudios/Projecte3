#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API ShopMenuManager : public Script {
public:
	ShopMenuManager();
	~ShopMenuManager();
	void Start() override;
	void Update() override;

	std::string buttonName = "Hi";

private:

	C_UI_Button* button;
};

SCRIPTS_FUNCTION ShopMenuManager* CreateShopMenuManager()
{
	ShopMenuManager* script = new ShopMenuManager();
	INSPECTOR_STRING(script->buttonName);
	return script;
}