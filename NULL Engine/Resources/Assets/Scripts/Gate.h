#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;

class SCRIPTS_API Gate : public Script {
public:
	Gate();
	~Gate();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void Unlock();

	std::string gameManagerName;
	bool isLocked = true;
private:
	GameObject* gameManager;
};

SCRIPTS_FUNCTION Gate* CreateGate();