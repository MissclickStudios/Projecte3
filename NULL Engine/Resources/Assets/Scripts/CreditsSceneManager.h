#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;
class C_AudioSource;
class GameManager;

class SCRIPTS_API CreditsSceneManager : public Script {
public:
	CreditsSceneManager();
	~CreditsSceneManager();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	GameObject* creditsPlane = nullptr;
	float moveSpeed = 1.f;

private:
	GameManager* gameManager = nullptr;
};

SCRIPTS_FUNCTION CreditsSceneManager* CreateCreditsSceneManager();
