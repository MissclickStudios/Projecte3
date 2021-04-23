#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_2DAnimator;

class SCRIPTS_API HUDManager : public Script {
public:
	HUDManager();
	~HUDManager();
	void Start() override;
	void Update() override;

	std::string dashImageName = "Hi";
	std::string creditsImageName = "Hi";
	std::string primaryWeaponImageName = "Hi";
	std::string secondaryWeaponImageName = "Hi";
	std::string mandoImageName = "Hi";

	std::string playerName = "Hi";

private:
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* mandoImage;

	GameObject* player;
};

SCRIPTS_FUNCTION HUDManager* CreateHUDManager() {
	HUDManager* script = new HUDManager();
	INSPECTOR_STRING(script->dashImageName);
	INSPECTOR_STRING(script->creditsImageName);
	INSPECTOR_STRING(script->primaryWeaponImageName);
	INSPECTOR_STRING(script->secondaryWeaponImageName);
	INSPECTOR_STRING(script->mandoImageName);
	INSPECTOR_STRING(script->playerName);
	return script;
}