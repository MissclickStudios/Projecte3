#include "GroundItem.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_Camera3D.h"
#include "M_UISystem.h"
#include "M_ResourceManager.h"
#include "R_Texture.h"

#include "GameManager.h"
#include "GameObject.h"
#include "C_Transform.h"
#include "C_Material.h"
#include "C_Canvas.h"
#include "C_ParticleSystem.h"
#include "Emitter.h"
#include "C_AudioSource.h"

#include "Player.h"
#include "ItemMenuManager.h"

#include "Items.h"

GroundItem::GroundItem() : Object()
{
	baseType = ObjectType::GROUND_ITEM;
}

GroundItem::~GroundItem()
{
}

void GroundItem::Awake()
{
	GameObject* menuGameObject = App->scene->GetGameObjectByName(itemMenuName.c_str());
	if (menuGameObject != nullptr)
		itemMenu = (ItemMenuManager*)menuGameObject->GetScript("ItemMenuManager");

	menuGameObject = App->scene->GetGameObjectByName("Game Manager");
	if (menuGameObject != nullptr)
		gameManager = (GameManager*)menuGameObject->GetScript("GameManager");

	player = App->scene->GetGameObjectByName(playerName.c_str());

	for (uint i = 0; i < gameObject->components.size(); ++i) // CANT GETCOMPONENT() OF PARTICLE SYSTEM
	{
		if (gameObject->components[i]->GetType() == ComponentType::PARTICLE_SYSTEM)
			particle = (C_ParticleSystem*)gameObject->components[i];
	}
	if (particle != nullptr)
		particle->StopSpawn();

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

	itemAudio = new C_AudioSource(gameObject);
}

void GroundItem::Update()
{
	if (player != nullptr && item != nullptr)
	{
		float playerY = player->transform->GetWorldPosition().y;
		float thisY = gameObject->transform->GetWorldPosition().y;

		if (abs(playerY - thisY) > 10)
			PickUp((Player*)player->GetScript("Player"));
	}

	float3 N, U, _U, R;
	float4x4 cameraTransform = App->camera->GetCurrentCamera()->GetOwner()->transform->GetWorldTransform();
	float3 direction = float3(cameraTransform.TranslatePart() - gameObject->transform->GetWorldPosition()).Normalized(); //normalized vector between the camera and gameobject position

	// Screem Aligned
	//N = cameraTransform.WorldZ().Normalized().Neg();	// N is the inverse of the camera +Z
	//U = cameraTransform.WorldY().Normalized();			// U is the up vector from the camera (already perpendicular to N)
	//R = U.Cross(N).Normalized();						// R is the cross product between  U and N
	// World Aligned
	N = direction;										// N is the direction
	_U = cameraTransform.WorldY().Normalized();			// _U is the up vector form the camera, only used to calculate R
	R = _U.Cross(N).Normalized();						// R is the cross product between U and N
	U = N.Cross(R).Normalized();						// U is the cross product between N and R

	float3x3 result = float3x3(R, U, N);

	gameObject->transform->SetWorldRotation(result.ToQuat());
}

void GroundItem::CleanUp()
{
	if (item != nullptr)
		delete item;

	if (itemAudio != nullptr)
		delete itemAudio;
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

	itemMenu->SetItem(this);
}

void GroundItem::PickUp(Player* player)
{
	player->SubtractCredits((int)((float)item->price * player->priceModifier));
	item->PickUp(player);
	if (item->toSave)
		player->AddItem(item->data);
	Deactivate();

	if (itemAudio != nullptr)
	{
		if (item->price == 0)
		{
			itemAudio->SetEvent("item_pickup");
			itemAudio->PlayFx(itemAudio->GetEventId());
		}
		else
		{
			itemAudio->SetEvent("item_buy");
			itemAudio->PlayFx(itemAudio->GetEventId());
		}
	}

	//pick item game manager
	if(gameManager != nullptr)
		gameManager->PickedItemUp();
}

bool GroundItem::AddItem(const std::vector<ItemData*> items, int num, bool toBuy)
{
	if (item != nullptr)
		delete item;

	ItemData* const itemData = Item::FindItem(items, num);
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

	if (particle != nullptr)
	{
		particle->StopSpawn();
		for (uint i = 0; i < particle->emitterInstances.size(); ++i)
		{
			std::string rarityName = "NONE";
			switch (item->rarity)
			{
			case ItemRarity::COMMON:
				rarityName = "COMMON";
				break;
			case ItemRarity::RARE:
				rarityName = "RARE";
				break;
			case ItemRarity::EPIC:
				rarityName = "EPIC";
				break;
			case ItemRarity::UNIQUE:
				rarityName = "UNIQUE";
				break;
			}
			if (particle->emitterInstances[i]->emitter->name == rarityName)
				particle->emitterInstances[i]->stopSpawn = false;
		}
	}

	return true;
}

bool GroundItem::AddItemByName(const std::vector<ItemData*> items, std::string name, ItemRarity rarity, bool toBuy)
{
	if (item != nullptr)
		delete item;

	ItemData* const itemData = Item::FindItem(items, name, rarity);
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
