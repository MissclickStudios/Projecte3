#include "GroundItem.h"

#include "Application.h"
#include "M_ResourceManager.h"
#include "R_Texture.h"

#include "GameObject.h"
#include "C_Material.h"

#include "Player.h"

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
	if (used || item == nullptr)
		return;
	Player* player = (Player*)object->GetScript("Player");
	if (player == nullptr || player->currency < item->price)
		return;

	used = true;
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
	return script;
}
