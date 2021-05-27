#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class Player;
class C_Canvas;
class GameManager;

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

	std::string mandoName = "Mando testbuild"; 
	std::string hubShopCanvasName = "HUBShopCanvas";

	float talkDistance = 10.f;

	bool menuOpen = false;
private:
	C_Canvas* hubShopCanvas = nullptr;
	Player* mando = nullptr;
	GameManager* gameManager = nullptr;
	HUBArmorerState state = HUBArmorerState::INACTIVE;

	
};

SCRIPTS_FUNCTION HUBArmorer* CreateHUBArmorer();