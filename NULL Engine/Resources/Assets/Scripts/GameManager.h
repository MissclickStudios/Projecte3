#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"

#include "Prefab.h"

class GameObject;
class C_UI_Text;

class SCRIPTS_API GameManager : public Script {
public:
	GameManager();
	~GameManager();
	//void Awake() override;
	void Start() override;
	//void PreUpdate()override;
	void Update() override;
	//void PostUpdate()override;
	//void CleanUp()override;

	std::string fpsText = "Enter a GO name";

	C_UI_Text* fpsCount;
};

SCRIPTS_FUNCTION GameManager* CreateGameManager() {
	GameManager* script = new GameManager();
	INSPECTOR_STRING(script->fpsText);
	return script;
}