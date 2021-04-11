#pragma once
#include <string>
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"

#include "Prefab.h"

class GameObject;

class SCRIPTS_API GameManager : public Script {
public:
	GameManager();
	~GameManager();
	/*void Awake() override;
	void Start() override;
	void PreUpdate()override;
	void Update() override;
	void PostUpdate()override;
	void CleanUp()override;*/
};

SCRIPTS_FUNCTION GameManager* CreateGameManager() {
	GameManager* script = new GameManager();
	return script;
}