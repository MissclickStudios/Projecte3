#include "ItemMenuManager.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"

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

	gameObject = App->scene->GetGameObjectByName(playerName.c_str());
	if (gameObject != nullptr)
		player = (Player*)gameObject->GetScript("Player");
}

void ItemMenuManager::Update()
{
	if (item != nullptr)
	{
		if (buyButton != nullptr && buyButton->GetState() == UIButtonState::PRESSEDIN)
		{
			if (player->currency >= item->item->price)
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
	else
		App->uiSystem->RemoveActiveCanvas(canvas);
}

void ItemMenuManager::SetItem(GroundItem* item)
{
	this->item = item;

	nameText->SetText(this->item->item->name.c_str());
	descriptionText->SetText(this->item->item->description.c_str());

	if (this->item->item->price > 0)
	{
		std::string text = "Price: ";
		text += std::to_string(this->item->item->price);
		text += "      Press Enter/A to pick up";
		priceText->SetText(text.c_str());
	}
	else
		priceText->SetText("Press Enter/A to pick up");

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