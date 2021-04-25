#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"


class GameObject;
class C_2DAnimator;
class Player;

class SCRIPTS_API HUDManager : public Script {
public:
	HUDManager();
	~HUDManager();

	void Start() override;
	void Update() override;

	std::string mandoImageName = "Mando";
	std::string secondaryWeaponImageName = "SecodaryWeapon";
	std::string primaryWeaponImageName = "PrimaryWeapon";
	std::string dashImageName = "Dash";
	std::string creditsImageName = "Credits";
	std::string playerName = "Mandalorian";

	std::string debugMenuCanvasName = "DebugMenuCanvas";
	std::string hubShopCanvasName = "hubShopCanvas";
	std::string hudCanvasName = "hubShopCanvas";
private:
	C_2DAnimator* mandoImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;

	GameObject* playerObject;
	Player* player;

	C_Canvas* debugMenuCanvas = nullptr;
	C_Canvas* hubShopCanvas = nullptr;
	C_Canvas* hudCanvas = nullptr;
};

SCRIPTS_FUNCTION HUDManager* CreateHUDManager() {
	HUDManager* script = new HUDManager();
	INSPECTOR_STRING(script->mandoImageName);
	INSPECTOR_STRING(script->secondaryWeaponImageName);
	INSPECTOR_STRING(script->primaryWeaponImageName);
	INSPECTOR_STRING(script->dashImageName);
	INSPECTOR_STRING(script->creditsImageName);
	INSPECTOR_STRING(script->playerName);
	INSPECTOR_STRING(script->debugMenuCanvasName);
	INSPECTOR_STRING(script->hubShopCanvasName);
	INSPECTOR_STRING(script->hudCanvasName);
	return script;
}