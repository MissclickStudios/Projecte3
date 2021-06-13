#include "ItemMenuManager.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "M_ResourceManager.h"
#include "R_Texture.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"
#include "C_UI_Image.h"
#include "C_Material.h"

#include "GroundItem.h"
#include "Items.h"

ItemMenuManager::ItemMenuManager()
{
}

ItemMenuManager::~ItemMenuManager()
{
}

void ItemMenuManager::Start()
{
	GameObject* gameObject = App->scene->GetGameObjectByName(canvasName.c_str());
	if (gameObject != nullptr)
		canvas = gameObject->GetComponent<C_Canvas>();

	gameObject = App->scene->GetGameObjectByName(buyButtonName.c_str());
	if (gameObject != nullptr)
		buyButton = gameObject->GetComponent<C_UI_Button>();

	gameObject = App->scene->GetGameObjectByName(nameTextName.c_str());
	if (gameObject != nullptr)
		nameText = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(descriptionTextName.c_str());
	if (gameObject != nullptr)
		descriptionText = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(priceTextName.c_str());
	if (gameObject != nullptr)
		priceText = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(rarityTextName.c_str());
	if (gameObject != nullptr)
		rarityText = gameObject->GetComponent<C_UI_Text>();

	gameObject = App->scene->GetGameObjectByName(itemImageName.c_str());
	if (gameObject != nullptr)
	{
		itemImage = gameObject->GetComponent<C_UI_Image>();
		itemMaterial = gameObject->GetComponent<C_Material>();
	}

	gameObject = App->scene->GetGameObjectByName(playerName.c_str());
	if (gameObject != nullptr)
		player = (Player*)gameObject->GetScript("Player");

	atlasTexture = (R_Texture*)App->resourceManager->GetResource<R_Texture>("Assets/Textures/Ui/Atlas.png");
}

void ItemMenuManager::Update()
{
	if (item != nullptr)
	{
		if (buyButton != nullptr && buyButton->GetState() == UIButtonState::PRESSEDIN)
		{
			if (player->credits >= (int)((float)item->item->price * player->priceModifier))
			{
				item->PickUp(player);
				item = nullptr;
				App->uiSystem->RemoveActiveCanvas(canvas);
			}
		}
		else
		{
			float3 playerPosition = player->transform->GetWorldPosition();
			float3 itemPosition = item->gameObject->transform->GetWorldPosition();

			float distance = playerPosition.Distance(itemPosition);
			if (distance >= closeMenuThreshold)
			{
				item = nullptr;
				App->uiSystem->RemoveActiveCanvas(canvas);
			}
		}
	}
	else if (weapon.uid != NULL)
	{
		if (buyButton != nullptr && buyButton->GetState() == UIButtonState::PRESSEDIN)
		{
			player->EquipWeapon(weapon);
			weapon = Prefab();
			weaponPosition = float3::zero;

			App->uiSystem->RemoveActiveCanvas(canvas);
		}
		else
		{
			float3 playerPosition = player->transform->GetWorldPosition();

			float distance = playerPosition.Distance(weaponPosition);
			if (distance >= closeMenuThreshold)
			{
				weapon = Prefab();
				weaponPosition = float3::zero;

				App->uiSystem->RemoveActiveCanvas(canvas);
			}
		}
	}
	else
		App->uiSystem->RemoveActiveCanvas(canvas);
}

void ItemMenuManager::CleanUp()
{
	if (currentItemTexture)
		App->resourceManager->FreeResource(currentItemTexture->GetUID());
	if(atlasTexture)
		App->resourceManager->FreeResource(atlasTexture->GetUID());
}

void ItemMenuManager::SetItem(GroundItem* item)
{
	if (weapon.uid == NULL && this->item == nullptr)
	{
		this->item = item;

		nameText->SetText(this->item->item->name.c_str());
		descriptionText->SetText(this->item->item->description.c_str());

		if (this->item->item->price > 0)
		{
			std::string text;
			if (player != nullptr)
			 text += std::to_string((int)((float)this->item->item->price * player->priceModifier));
			//text += "      Enter/A";
			priceText->SetText(text.c_str());
		}
		//else
		//	priceText->SetText("      Enter/A");

		switch (this->item->item->rarity)
		{
		case ItemRarity::COMMON:
			rarityText->SetText("COMMON");
			rarityText->SetColor(COMMON_COLOR);
			break;
		case ItemRarity::RARE:
			rarityText->SetText("RARE");
			rarityText->SetColor(RARE_COLOR);
			break;
		case ItemRarity::EPIC:
			rarityText->SetText("EPIC");
			rarityText->SetColor(EPIC_COLOR);
			break;
		case ItemRarity::UNIQUE:
			rarityText->SetText("UNIQUE");
			rarityText->SetColor(UNIQUE_COLOR);
			break;
		}

		if (itemMaterial != nullptr)
			if (item->item->texturePath != "")
			{
				//if (currentItemTexture)
				//	App->resourceManager->FreeResource(currentItemTexture->GetUID());
				currentItemTexture = (R_Texture*)App->resourceManager->GetResource<R_Texture>(item->item->texturePath.c_str());
				if (currentItemTexture != nullptr) 
				{
					itemMaterial->SetTexture(currentItemTexture);
					C_UI_Image* currImage = itemMaterial->GetOwner()->GetComponent<C_UI_Image>();
					currImage->SetTextureCoordinates(0, 0, 512, 512);
					currImage->SetRect({ -0.9, -0.585, 0.18, 0.23 });
				}
			}

		App->uiSystem->PushCanvas(canvas);
	}
}

void ItemMenuManager::SetWeapon(Prefab weapon, float3 position, std::string name, std::string description)
{
	if (weapon.uid == NULL)
		return;

	if (this->weapon.uid == NULL && item == nullptr)
	{
		nameText->SetText(name.c_str());
		descriptionText->SetText(description.c_str());
		priceText->SetText("");
		rarityText-> SetText("");

		weaponPosition = position;
		this->weapon = weapon;

		itemMaterial->SetTexture(atlasTexture);
		C_UI_Image* currImage = itemMaterial->GetOwner()->GetComponent<C_UI_Image>();
		currImage->SetTextureCoordinates(-340, -225, 121, 84);
		currImage->SetRect({ -0.875f, -0.545f, 0.135f, 0.155f });
		App->uiSystem->PushCanvas(canvas);
	}
}

SCRIPTS_FUNCTION ItemMenuManager* CreateItemMenuManager()
{
	ItemMenuManager* script = new ItemMenuManager();

	INSPECTOR_STRING(script->canvasName);
	INSPECTOR_STRING(script->buyButtonName);
	INSPECTOR_STRING(script->nameTextName);
	INSPECTOR_STRING(script->descriptionTextName);
	INSPECTOR_STRING(script->priceTextName);
	INSPECTOR_STRING(script->playerName);

	return script;
}