#pragma once
#include "Object.h"
#include "ScriptMacros.h"

#include "Prefab.h"

#include <vector>
#include <string>

class GameManager;
class C_AudioSource;

class SCRIPTS_API Chest : public Object ALLOWED_INHERITANCE
{
public:

	Chest();
	virtual ~Chest();

	void Awake() override;
	void Update() override;
	void CleanUp() override;

	void OnPause() override;
	void OnResume() override;

	void OnCollisionEnter(GameObject* object) override;
	
	std::string gameManagerName = "Game Manager";
	std::string playerName = "Mandalorian";

	int rollAttempts = 100;

	float stimPackThreshold = 2.0f;
	int stimPackChance = 60;

	Prefab itemPrefab;

protected:

	bool open = false;
	C_AudioSource* chestAudio = nullptr;

	GameManager* gameManager = nullptr;
};

SCRIPTS_FUNCTION Chest* CreateChest();