#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;

class SCRIPTS_API Gate : public Script {
public:
	Gate();
	~Gate();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;
	GameObject* gameManager = nullptr;
};

SCRIPTS_FUNCTION Gate* CreateGate() {
	Gate* script = new Gate();
	INSPECTOR_GAMEOBJECT(script->gameManager);
	return script;
}