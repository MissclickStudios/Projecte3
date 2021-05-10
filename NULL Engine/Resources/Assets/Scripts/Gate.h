#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_AudioSource;

class SCRIPTS_API Gate : public Script {
public:
	Gate();
	~Gate();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void Unlock();

	float quoteTimer = 0.f;
	float quoteDelay = 5.f;

	std::string gameManagerName;
	bool isLocked = true;

	C_AudioSource* quote = nullptr;
private:
	GameObject* gameManager = nullptr;
};

SCRIPTS_FUNCTION Gate* CreateGate();