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
	GameObject* managerObject = App->scene->GetGameObjectByName(gameManagerName.c_str());
	if (managerObject == nullptr)
		return;
	gameManager = (GameManager*)managerObject->GetScript("GameManager");
	if (gameManager == nullptr)
		return;

	//GameObject* playerObject = App->scene->GetGameObjectByName(playerName.c_str());
	//Player* player = (Player*)managerObject->GetScript("Player");
	//if (player != nullptr && player->health <= 2.0f)
	//{
	//		FOR LATER
	//}

	std::vector<GameObject*>* gameObjects = App->scene->GetGameObjects();
	for (uint i = 0; i < gameObjects->size(); ++i)
	{
		GroundItem* item = (GroundItem*)GetObjectScript((*gameObjects)[i], ObjectType::ITEM);
		if (item == nullptr)
			continue;

		uint searches = 0;
		while (searches < 10) // RIP while(true) loop, you'll be missed. 16/5/21 (13:00) - 16/5/21 (13:05)
		{
			uint num = Random::LCG::GetBoundedRandomUint(0, 100);
			LOG("%d", num);
			if (item->AddItem(gameManager->GetChestItemPool(), num))
			{
				if (item->item != nullptr)
				{
					std::string* name = &item->item->name;

					bool found = false;
					for (uint n = 0; n < itemNames.size(); ++n)
						if (itemNames[n] == *name)
						{
							found = true;
							break;
						}

					if (!found)
					{
						itemNames.push_back(*name);
						break;
					}
				}
			}
			searches++;
		}
	}
}

void InGameShop::Update()
{
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
	INSPECTOR_VECTOR_STRING(script->itemNames);

	return script;
}
