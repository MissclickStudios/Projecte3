#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_UI_Button;
class C_UI_Text;
class C_Canvas;

class SCRIPTS_API DebugMenuManager : public Script {
public:
	DebugMenuManager();
	~DebugMenuManager();
	void Start() override;
	void Update() override;

	
	int beskarToAdd = 10;
	int creditsToAdd = 10;
	int healthHealed = 1;

	std::string godModeName = "GodMode";
	std::string roomSkipName = "SkipRoom";
	std::string addHealthName = "HealMando";
	std::string getCreditsName = "GetCredits";
	std::string getBeskarName = "GetBeskarIngots";
	std::string spawnBlurggName = "SpawnBlurgg";
	std::string spawnTrooperName = "SpawnTrooper";
	std::string gameManagerName = "GameManager";
	std::string mandoName = "Mandalorian";

	std::string fpsTextName = "fpsText";

	std::string debugMenuCanvasName = "DebugMenuCanvas";

	//- unlock items missing
	Prefab blurgg;
	Prefab trooper;
private:
	C_UI_Button* godMode = nullptr;
	C_UI_Button* roomSkip= nullptr;
	C_UI_Button* addHealth = nullptr;
	C_UI_Button* getCredits = nullptr;
	C_UI_Button* getBeskar = nullptr;
	C_UI_Button* spawnBlurgg = nullptr;
	C_UI_Button* spawnTrooper = nullptr;

	C_UI_Text* fpsText = nullptr;

	C_Canvas* debugMenuCanvas = nullptr;

	GameObject* gameManager = nullptr;
	GameObject* mando = nullptr;
};

SCRIPTS_FUNCTION DebugMenuManager* CreateDebugMenuManager() {
	DebugMenuManager* script = new DebugMenuManager();
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->debugMenuCanvasName);
	INSPECTOR_STRING(script->godModeName);
	INSPECTOR_STRING(script->roomSkipName);
	INSPECTOR_STRING(script->addHealthName);
	INSPECTOR_STRING(script->getCreditsName);
	INSPECTOR_STRING(script->getBeskarName);
	INSPECTOR_STRING(script->spawnBlurggName);
	INSPECTOR_STRING(script->spawnTrooperName);
	INSPECTOR_STRING(script->fpsTextName);

	INSPECTOR_PREFAB(script->blurgg);
	INSPECTOR_PREFAB(script->trooper);
	return script;
}