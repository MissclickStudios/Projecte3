#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"

#include "MathGeoLib/include/Math/float3.h"

#include <string>

class C_Canvas;
class C_UI_Image;
class C_UI_Text;

class Player;
class ItemData;

class SCRIPTS_API PlayerItemMenuManager : public Script
{
public:

	PlayerItemMenuManager();
	~PlayerItemMenuManager();

	void Start() override;
	void Update() override;

	Prefab itemFramePrefab;
	Prefab rarityDisplayPrefab;
	std::string playerName = "Mandalorian";
	std::string canvasName = "PlayerItemMenu";

	float menuX = 0.75f;
	float menuY = 0.4f;
	float separation = 0.1f;
	float spacing = 0.2f;

	C_Canvas* canvas = nullptr;

private:

	std::vector<C_UI_Image*> images;
	std::vector<std::pair<bool, ItemData*>> lastItems;

	C_UI_Text* nameText = nullptr;

	Player* player = nullptr;
};

SCRIPTS_FUNCTION PlayerItemMenuManager* CreatePlayerItemMenuManager();

