#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_Canvas;

enum class HUBArmorerState: char {
	ACTIVE,
	INACTIVE,
};

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
	C_Canvas* hubShopCanvas = nullptr;
	GameObject* mando = nullptr;
	HUBArmorerState state = HUBArmorerState::INACTIVE;
};

SCRIPTS_FUNCTION HUBArmorer* CreateHUBArmorer() {
	HUBArmorer* script = new HUBArmorer();
	INSPECTOR_STRING(script->hubShopCanvasName);
	INSPECTOR_STRING(script->HUDCanvasName);
	INSPECTOR_STRING(script->mandoName);
	return script;
}