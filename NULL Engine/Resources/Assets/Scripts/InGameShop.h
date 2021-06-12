#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include <vector>
#include <string>

class GameManager;

class SCRIPTS_API InGameShop : public Script 
{
public:

	InGameShop();
	~InGameShop();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	std::string playerName = "Mandalorian";
	std::string gameManagerName = "Game Manager";
	int rollAttempts = 100;

	std::vector<std::string> itemNames;
	std::string stimPackName = "StimPack";

private:

	GameManager* gameManager = nullptr;

	int counter = 0;
};

SCRIPTS_FUNCTION InGameShop* CreateInGameShop();