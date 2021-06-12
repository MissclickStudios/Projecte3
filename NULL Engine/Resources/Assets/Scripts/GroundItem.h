#pragma once
#include "Object.h"
#include "ScriptMacros.h"

#include <vector>
#include <string>

#include "ItemRarity.h"

class C_Material;
class C_ParticleSystem;
class ItemMenuManager;
class GameManager;
class C_AudioSource;

class Item;
struct ItemData;

class Player;

class SCRIPTS_API GroundItem : public Object ALLOWED_INHERITANCE
{
public:

	GroundItem();
	virtual ~GroundItem();

	void Awake() override;
	void Update() override;
	void CleanUp() override;

	void OnPause() override;
	void OnResume() override;

	void OnTriggerRepeat(GameObject* object) override;

	void PickUp(Player* player);

	bool AddItem(const std::vector<ItemData*> items, int num, bool toBuy = true);
	bool AddItemByName(const std::vector<ItemData*> items, std::string name, ItemRarity rarity, bool toBuy = true);

	Item* item = nullptr;

	std::string itemMenuName = "Item Menu";
	std::string playerName = "Mandalorian";

protected:

	C_Material* material = nullptr;
	C_ParticleSystem* particle = nullptr;
	C_AudioSource* itemAudio = nullptr;

	ItemMenuManager* itemMenu = nullptr;

	GameManager* gameManager = nullptr;
	GameObject* player = nullptr;
};

SCRIPTS_FUNCTION GroundItem* CreateGroundItem();