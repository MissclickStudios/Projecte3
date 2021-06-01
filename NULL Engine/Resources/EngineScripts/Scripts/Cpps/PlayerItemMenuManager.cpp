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
	while (itemFrames.size() != 0)
	{
		(*itemFrames.begin())->GetOwner()->toDelete = true;
		itemFrames.erase(itemFrames.begin());
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
		while (itemFrames.size() != 0)
		{
			(*itemFrames.begin())->GetOwner()->toDelete = true;
			itemFrames.erase(itemFrames.begin());
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
				if (x == 0.0f)
					x += separation;
				else
				{
					x = 0.0f;
					y -= spacing;
				}
				switch ((*items)[i].second->rarity)
				{
				case ItemRarity::COMMON:
					itemFrame->SetColor(102, 255, 102, 200);
					break;
				case ItemRarity::RARE:
					itemFrame->SetColor(102, 255, 255, 200);
					break;
				case ItemRarity::EPIC:
					itemFrame->SetColor(255, 102, 255, 200);
					break;
				case ItemRarity::UNIQUE:
					itemFrame->SetColor(255, 255, 102, 200);
					break;
				}

				C_Material* material = itemFrameGameObject->GetComponent<C_Material>();
				R_Texture* texture = App->resourceManager->GetResource<R_Texture>((*items)[i].second->texturePath.c_str());
				material->SetTexture(texture);

				itemFrames.push_back(itemFrame);
			}
		}
	}
	
	lastItems = *items;
}

SCRIPTS_FUNCTION PlayerItemMenuManager* CreatePlayerItemMenuManager()
{
	PlayerItemMenuManager* script = new PlayerItemMenuManager();

	INSPECTOR_PREFAB(script->itemFramePrefab);
	INSPECTOR_STRING(script->playerName);

	INSPECTOR_DRAGABLE_FLOAT(script->menuX);
	INSPECTOR_DRAGABLE_FLOAT(script->menuY);
	INSPECTOR_DRAGABLE_FLOAT(script->separation);
	INSPECTOR_DRAGABLE_FLOAT(script->spacing);

	return script;
}
