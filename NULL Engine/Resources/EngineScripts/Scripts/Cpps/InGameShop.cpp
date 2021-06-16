#include "InGameShop.h"

#include "Application.h"
#include "M_Scene.h"

#include "GameObject.h"

#include "GameManager.h"
#include "Object.h"
#include "GroundItem.h"

#include "Items.h"
#include "Player.h"

#include "Random.h"

#include "Log.h"

InGameShop::InGameShop()
{
}

InGameShop::~InGameShop()
{
}

void InGameShop::Start()
{
}

void InGameShop::Update()
{
	if (counter < 5)
	{
		counter++;
		return;
	}
	if (gameManager != nullptr)
		return;

	GameObject* managerObject = App->scene->GetGameObjectByName(gameManagerName.c_str());
	if (managerObject == nullptr)
		return;
	gameManager = (GameManager*)managerObject->GetScript("GameManager");
	if (gameManager == nullptr)
		return;

	GameObject* playerObject = App->scene->GetGameObjectByName(playerName.c_str());
	if (playerObject == nullptr)
		return;
	Player* player = (Player*)playerObject->GetScript("Player");
	if (player == nullptr)
		return;

	const std::vector<std::pair<bool, ItemData*>>* const playerItems = player->GetItems();

	std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
	for (uint i = 0; i < gameObjects->size(); ++i)
	{
		GroundItem* item = (GroundItem*)GetObjectScript((*gameObjects)[i], ObjectType::GROUND_ITEM);
		if (item == nullptr)
			continue;

		std::string name = (*gameObjects)[i]->GetName();
		if (name == stimPackName)
		{
			LOG("health: %f < maxHealth: %f", player->health, player->MaxHealth());
			if (player->health < player->MaxHealth())
				item->AddItemByName(gameManager->GetShopItemPool(), "Stim Pack", ItemRarity::COMMON);
			else
				item->Deactivate();
			continue;
		}

		uint searches = 0;
		while (searches < rollAttempts) // RIP while(true) loop, you'll be missed. 16/5/21 (13:00) - 16/5/21 (13:05)
		{
			uint num = Random::LCG::GetBoundedRandomUint(0, 100);
			if (item->AddItem(gameManager->GetShopItemPool(), num))
			{
				std::string name = item->item->name;

				bool found = false;
				for (uint n = 0; n < itemNames.size(); ++n)
				{
					if (itemNames[n] == name)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					for (uint i = 0; i < playerItems->size(); ++i)
					{
						ItemData* playerItem = (*playerItems)[i].second;
						if (name == playerItem->name && item->item->rarity <= playerItem->rarity)
						{
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					itemNames.push_back(name);
					break;
				}
			}
			searches++;
		}
		if (searches >= rollAttempts)
			item->Deactivate();
	}
}


void InGameShop::CleanUp()
{
	gameManager = nullptr;
	itemNames.clear();
}

SCRIPTS_FUNCTION InGameShop* CreateInGameShop()
{
	InGameShop* script = new InGameShop();

	INSPECTOR_STRING(script->playerName);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->stimPackName);
	INSPECTOR_VECTOR_STRING(script->itemNames);
	INSPECTOR_DRAGABLE_INT(script->rollAttempts);

	return script;
}
