#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;

class SCRIPTS_API HUBArmorer : public Script {
public:
	HUBArmorer();
	~HUBArmorer();
	void Start() override;
	void Update() override;

	std::string hubShopCanvasName = "HUBShopCanvas";
	std::string HUDCanvasName = "Canvas";
	std::string mandoName = "Mando testbuild";
	float talkDistance = 10.f;
private:
	GameObject* hubShopCanvas = nullptr;
	GameObject* HUDCanvas = nullptr;
	GameObject* mando = nullptr;
};

SCRIPTS_FUNCTION HUBArmorer* CreateHUBArmorer() {
	HUBArmorer* script = new HUBArmorer();
	INSPECTOR_STRING(script->hubShopCanvasName);
	INSPECTOR_STRING(script->HUDCanvasName);
	INSPECTOR_STRING(script->mandoName);
	return script;
}