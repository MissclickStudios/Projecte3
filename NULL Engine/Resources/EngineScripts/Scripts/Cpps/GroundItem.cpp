#include "GroundItem.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"
#include "M_ResourceManager.h"
#include "R_Texture.h"

#include "GameObject.h"
#include "C_Material.h"
#include "C_Canvas.h"

#include "Player.h"
#include "ItemMenuManager.h"

#include "Items.h"

GroundItem::GroundItem() : Object()
{
	baseType = ObjectType::ITEM;
}

GroundItem::~GroundItem()
{
}

void GroundItem::Awake()
{
	GameObject* gameObject = App->scene->GetGameObjectByName(itemMenuName.c_str());
	if (gameObject != nullptr)
		itemMenu = (ItemMenuManager*)gameObject->GetScript("ItemMenuManager");

	for (uint i = 0; i < gameObject->childs.size(); ++i)
	{
		C_Material* maybe = gameObject->childs[i]->GetComponent<C_Material>();
		if (maybe != nullptr)
		{
			material = maybe;
			material->SetTakeDamage(true);
			break;
		}
	}
}

void GroundItem::Update()
{
}

void GroundItem::CleanUp()
{
}

void GroundItem::OnPause()
{
}

void GroundItem::OnResume()
{
}

void GroundItem::OnTriggerRepeat(GameObject* object)
{
	if (item == nullptr)
		return;

	if (this != itemMenu->GetItem()) // WHAT HAPPENS IF I PUSH A PUSHED CANVAS?
	{
		itemMenu->SetItem(this);
		App->uiSystem->PushCanvas(itemMenu->canvas);
	}
}

void GroundItem::PickUp(Player* player)
{
	player->currency -= item->price;
	item->PickUp(player);
	Deactivate();
}

bool GroundItem::AddItem(const std::vector<ItemData*> items, int num, bool toBuy)
{
	const ItemData* const itemData = Item::FindItem(items, num);
	if (itemData == nullptr)
		return false;

	item = Item::CreateItem(itemData, toBuy);
	if (item == nullptr)
		return false;

	if (material != nullptr)
		if (itemData->texturePath != "")
		{
			R_Texture* texture = App->resourceManager->GetResource<R_Texture>(itemData->texturePath.c_str());
			if (texture != nullptr)
				material->SetTexture(texture);
		}

	return true;
}

bool GroundItem::AddItemByName(const std::vector<ItemData*> items, std::string name, ItemRarity rarity, bool toBuy)
{
	const ItemData* const itemData = Item::FindItem(items, name, rarity);
	if (itemData == nullptr)
		return false;

	item = Item::CreateItem(itemData, toBuy);
	if (item == nullptr)
		return false;

	if (material != nullptr)
		if (itemData->texturePath != "")
		{
			R_Texture* texture = App->resourceManager->GetResource<R_Texture>(itemData->texturePath.c_str());
			if (texture != nullptr)
				material->SetTexture(texture);
		}

	return true;
}

SCRIPTS_FUNCTION GroundItem* CreateGroundItem()
{
	GroundItem* script = new GroundItem();

	INSPECTOR_STRING(script->itemMenuName);

	return script;
}
