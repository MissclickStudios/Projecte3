#include "Chest.h"

#include "Application.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "GameManager.h"
#include "Player.h"
#include "GroundItem.h"
#include "Items.h"

#include "Random.h"

Chest::Chest() : Object()
{
}

Chest::~Chest()
{
}

void Chest::Awake()
{
}

void Chest::Update()
{
}

void Chest::CleanUp()
{
}

void Chest::OnPause()
{
}

void Chest::OnResume()
{
}

void Chest::OnTriggerRepeat(GameObject* object)
{
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

	if (itemPrefab.uid == NULL)
		return;
	GameObject* groundItem = App->resourceManager->LoadPrefab(itemPrefab.uid, App->scene->GetSceneRoot());
	if (groundItem == nullptr)
		return;
	float3 position = gameObject->transform->GetWorldPosition();
	groundItem->transform->SetWorldPosition(position);

	GroundItem* item = (GroundItem*)groundItem->GetScript("GroundItem");
	if (item == nullptr)
		return;
	item->Awake();

	float healthDiff = player->MaxHealth() - player->health;
	if (healthDiff != 0.0f)
	{
		int chance = stimPackChance * (healthDiff / player->MaxHealth());
		uint num = Random::LCG::GetBoundedRandomUint(0, 100);
		if (num <= chance && item->AddItemByName(gameManager->GetChestItemPool(), "Stim Pack", ItemRarity::COMMON, false))
		{
			Deactivate();
			return;
		}
	}

	uint searches = 0;
	while (searches < rollAttempts)
	{
		uint num = Random::LCG::GetBoundedRandomUint(0, 100);
		if (item->AddItem(gameManager->GetChestItemPool(), num, false) && item->item != nullptr)
		{
			std::string name = item->item->name;

			bool found = false;
			for (uint i = 0; i < playerItems->size(); ++i)
			{
				ItemData* playerItem = (*playerItems)[i].second;
				if (name == playerItem->name && item->item->rarity <= playerItem->rarity)
				{
					found = true;
					break;
				}
			}

			if (!found)
				break;
		}
		searches++;
	}
	if (searches >= rollAttempts)
		item->Deactivate();

	Deactivate();
}

SCRIPTS_FUNCTION Chest* CreateChest()
{
	Chest* script = new Chest();
	
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->playerName);
	INSPECTOR_DRAGABLE_INT(script->rollAttempts);

	INSPECTOR_DRAGABLE_FLOAT(script->stimPackThreshold);
	INSPECTOR_DRAGABLE_INT(script->stimPackChance);

	INSPECTOR_PREFAB(script->itemPrefab);

	return script;
}
