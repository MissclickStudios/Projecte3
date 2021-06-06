#include "PlayerItemMenuManager.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"
#include "M_ResourceManager.h"
#include "R_Texture.h"

#include "GameObject.h"
#include "C_UI_Image.h"
#include "C_Canvas.h"
#include "C_Material.h"

#include "Items.h"

#include "Player.h"

PlayerItemMenuManager::PlayerItemMenuManager()
{
}

PlayerItemMenuManager::~PlayerItemMenuManager()
{
	while (images.size() != 0)
	{
		(*images.begin())->GetOwner()->toDelete = true;
		images.erase(images.begin());
	}
}

void PlayerItemMenuManager::Start()
{
	GameObject* go = App->scene->GetGameObjectByName(playerName.c_str());
	if (go != nullptr)
		player = (Player*)go->GetScript("Player");

	go = App->scene->GetGameObjectByName(canvasName.c_str());
	if (go != nullptr)
		canvas = go->GetComponent<C_Canvas>();

	App->uiSystem->PushCanvas(canvas);
}

void PlayerItemMenuManager::Update()
{
	if (!strcmp(App->scene->GetCurrentScene(), "HUB"))
		return;
	if (itemFramePrefab.uid == NULL)
		return;
	if (player == nullptr)
		return;

	bool secondaryGun = player->GetUsingSecondaryGun();
	const std::vector<std::pair<bool, ItemData*>>* const items = player->GetItems();

	bool reload = false;

	if (items->size() != lastItems.size())
		reload = true;
	else
		for (uint i = 0; i < items->size(); ++i)
		{

			bool weapon = (*items)[i].first;
			ItemData* data = (*items)[i].second;
			if (weapon != lastItems[i].first)
			{
				reload = true;
				break;
			}
			if (data->name != lastItems[i].second->name)
			{
				reload = true;
				break;
			}
			if (data->rarity != lastItems[i].second->rarity)
			{
				reload = true;
				break;
			}
		}

	if (reload)
	{
		while (images.size() != 0)
		{
			(*images.begin())->GetOwner()->toDelete = true;
			images.erase(images.begin());
		}

		float x = 0.0f;
		float y = 0.0f;
		for (uint i = 0; i < items->size(); ++i)
		{
			GameObject* itemFrameGameObject = App->resourceManager->LoadPrefab(itemFramePrefab.uid, canvas->GetOwner());
			if (itemFrameGameObject != nullptr)
			{
				C_UI_Image* itemFrame = itemFrameGameObject->GetComponent<C_UI_Image>();
				itemFrame->SetX(menuX + x);
				itemFrame->SetY(menuY + y);
				itemFrame->SetColor(255, 255, 255, 200);

				GameObject* rarityDisplayGameObject = App->resourceManager->LoadPrefab(rarityDisplayPrefab.uid, canvas->GetOwner());
				if (rarityDisplayGameObject != nullptr)
				{
					C_UI_Image* rarityDisplay = rarityDisplayGameObject->GetComponent<C_UI_Image>();
					rarityDisplay->SetX(menuX + x);
					rarityDisplay->SetY(menuY + y);

					switch ((*items)[i].second->rarity)
					{
					case ItemRarity::COMMON:
						rarityDisplay->SetColor(50, 255, 50, 200);
						break;
					case ItemRarity::RARE:
						rarityDisplay->SetColor(50, 255, 255, 200);
						break;
					case ItemRarity::EPIC:
						rarityDisplay->SetColor(255, 50, 255, 200);
						break;
					case ItemRarity::UNIQUE:
						rarityDisplay->SetColor(255, 255, 50, 200);
						break;
					}

					images.push_back(rarityDisplay);
				}

				if (x == 0.0f)
					x += separation;
				else
				{
					x = 0.0f;
					y -= spacing;
				}

				C_Material* material = itemFrameGameObject->GetComponent<C_Material>();
				R_Texture* texture = App->resourceManager->GetResource<R_Texture>((*items)[i].second->texturePath.c_str());
				material->SetTexture(texture);

				images.push_back(itemFrame);
			}
		}
	}
	
	lastItems = *items;
}

SCRIPTS_FUNCTION PlayerItemMenuManager* CreatePlayerItemMenuManager()
{
	PlayerItemMenuManager* script = new PlayerItemMenuManager();

	INSPECTOR_PREFAB(script->itemFramePrefab);
	INSPECTOR_PREFAB(script->rarityDisplayPrefab);
	INSPECTOR_STRING(script->playerName);

	INSPECTOR_DRAGABLE_FLOAT(script->menuX);
	INSPECTOR_DRAGABLE_FLOAT(script->menuY);
	INSPECTOR_DRAGABLE_FLOAT(script->separation);
	INSPECTOR_DRAGABLE_FLOAT(script->spacing);

	return script; 
}
