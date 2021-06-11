#include "Application.h"
#include "M_Scene.h"

#include "M_Input.h"
#include "M_UISystem.h"

#include "C_Animator.h"
#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"
#include "C_UI_Slider.h"

#include "GameManager.h"
#include "GameObject.h"
#include "Player.h"
#include "HUBArmorer.h"

#include "Items.h"

#define MAX_ITEM_LEVEL 3
#define MAX_ITEM_PRICE 4

HUBArmorer::HUBArmorer()
{
}

HUBArmorer::~HUBArmorer()
{
}

void HUBArmorer::Start()
{
	GameObject* obj = App->scene->GetGameObjectByName(hubShopCanvasName.c_str());
	if(obj != nullptr)
		hubShopCanvas = obj->GetComponent<C_Canvas>();

	obj = App->scene->GetGameObjectByName(mandoName.c_str());
	if (obj != nullptr)
		mando = (Player*)obj->GetScript("Player");

	obj = App->scene->GetGameObjectByName("Game Manager");
	if (obj != nullptr)
		gameManager = (GameManager*)obj->GetScript("GameManager");
	if (gameManager != nullptr)
		hubItems = gameManager->GetHubItemPool();

	obj = gameObject->FindChild("ArmorerModel");
	if (obj != nullptr)
		armorerAnimator = obj->GetComponent<C_Animator>();

	if (armorButtonObject != nullptr)
		armorButton = (C_UI_Button*)armorButtonObject->GetComponent<C_UI_Button>();
	if (bootsButtonObject != nullptr)
		bootsButton = (C_UI_Button*)bootsButtonObject->GetComponent<C_UI_Button>();
	if (ticketButtonObject != nullptr)
		ticketButton = (C_UI_Button*)ticketButtonObject->GetComponent<C_UI_Button>();
	if (bottleButtonObject != nullptr)
		bottleButton = (C_UI_Button*)bottleButtonObject->GetComponent<C_UI_Button>();
	if (armorSliderObject != nullptr) 
	{
		armorSlider = (C_UI_Slider*)armorSliderObject->GetComponent<C_UI_Slider>();
		if (armorSlider)
		{
			armorSlider->Hoverable(false);
			if(gameManager)
				armorSlider->InputValue(gameManager->armorLvl, MAX_ITEM_LEVEL);
		}
	}
	if (ticketSliderObject != nullptr) 
	{
		ticketSlider = (C_UI_Slider*)ticketSliderObject->GetComponent<C_UI_Slider>();
		if (ticketSlider)
		{
			ticketSlider->Hoverable(false);
			if (gameManager)
				ticketSlider->InputValue(gameManager->ticketLvl, MAX_ITEM_LEVEL);
		}
	}
	if (bootsSliderObject != nullptr)
	{
		bootsSlider = (C_UI_Slider*)bootsSliderObject->GetComponent<C_UI_Slider>();
		if (bootsSlider)
		{
			bootsSlider->Hoverable(false);
			if (gameManager)
				bootsSlider->InputValue(gameManager->bootsLvl, MAX_ITEM_LEVEL);
		}
	}
	if (bottleSliderObject != nullptr)
	{
		bottleSlider = (C_UI_Slider*)bottleSliderObject->GetComponent<C_UI_Slider>();
		if (bottleSlider)
		{
			bottleSlider->Hoverable(false);
			if (gameManager)
				bottleSlider->InputValue(gameManager->bottleLvl, MAX_ITEM_LEVEL);
		}
	}
	if (armorTextObject != nullptr)
	{
		armorPriceText = (C_UI_Text*)armorTextObject->GetComponent<C_UI_Text>();
		if (armorPriceText != nullptr)
			armorPriceText->SetText(std::to_string(gameManager->armorLvl + 1).c_str());
	}
	if (bootsTextObject != nullptr)
	{
		bootsPriceText = (C_UI_Text*)bootsTextObject->GetComponent<C_UI_Text>();
		if (bootsPriceText != nullptr)
			bootsPriceText->SetText(std::to_string(gameManager->bootsLvl + 1).c_str());
	}
	if (ticketTextObject != nullptr)
	{
		ticketPriceText = (C_UI_Text*)ticketTextObject->GetComponent<C_UI_Text>();
		if (ticketPriceText != nullptr)
			ticketPriceText->SetText(std::to_string(gameManager->ticketLvl + 1).c_str());
	}
	if (bottleTextObject != nullptr)
	{
		bottlePriceText = (C_UI_Text*)bottleTextObject->GetComponent<C_UI_Text>();
		if (bottlePriceText != nullptr)
			bottlePriceText->SetText(std::to_string(gameManager->bottleLvl + 1).c_str());
	}
	if (titleTextObject != nullptr)
		titleText = (C_UI_Text*)titleTextObject->GetComponent<C_UI_Text>();
	if (descriptionTextObject != nullptr)
		descriptionText = (C_UI_Text*)descriptionTextObject->GetComponent<C_UI_Text>();
	//if (creditTextObject != nullptr)
	//	creditsText = (C_UI_Text*)creditTextObject->GetComponent<C_UI_Text>();
	if (beskarTextObject != nullptr)
		beskarText = (C_UI_Text*)beskarTextObject->GetComponent<C_UI_Text>();


}

void HUBArmorer::Update()
{
	if (mando != nullptr)
	{
		switch (state)
		{
		case HUBArmorerState::NONE:

			if (armorerAnimator != nullptr)
				if (armorerAnimator->GetTrackAsPtr("Preview") != nullptr)
				{
					armorerAnimator->PlayClip("Preview", "Idle", 0.f);
					state = HUBArmorerState::INACTIVE;
				}

			break;
		case HUBArmorerState::INACTIVE:

			if (mando->transform->GetDistanceTo(gameObject->transform->GetLocalPosition()) <= talkDistance)
			{
				if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP ) // A/X button on controller
				{
					App->uiSystem->PushCanvas(hubShopCanvas);
					menuOpen = true;
					state = HUBArmorerState::ACTIVE;
					gameManager->TalkedToArmorer();

					if (armorerAnimator != nullptr)
						armorerAnimator->PlayClip("Preview", "Talk", 0.f);
				}
			}

			

			break;
		case HUBArmorerState::ACTIVE:

			if (mando->transform->GetDistanceTo(gameObject->transform->GetLocalPosition()) >= talkDistance || App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_UP) // B/Square button on controller
			{
				App->uiSystem->RemoveActiveCanvas(hubShopCanvas);
				mando->SetPlayerInteraction(InteractionType::NONE);
				menuOpen = false;
				state = HUBArmorerState::INACTIVE;

				if (armorerAnimator != nullptr)
					armorerAnimator->PlayClip("Preview", "Idle", 0.f);

				break;
			}

			UpdateMenu();
			
			break;
		default:
			state = HUBArmorerState::INACTIVE; break;
		}
	}
}

void HUBArmorer::UpdateMenu()
{
	if (mando != nullptr) {
		if (gameManager != nullptr)
		{
			if (armorButton != nullptr && armorButton->GetState() == UIButtonState::RELEASED)
			{
				if (mando->beskar > gameManager->armorLvl && gameManager->armorLvl + 1 < MAX_ITEM_PRICE)
				{
					gameManager->armorLvl = armorSlider->IncrementOneSquare();
					mando->SubtractBeskar(gameManager->armorLvl);
					armorPriceText->SetText(std::to_string(gameManager->armorLvl + 1).c_str());

					if (mando != nullptr)
					{
						ItemData* const itemData = Item::FindItem(hubItems, "Durasteel Reinforcement", (ItemRarity)gameManager->armorLvl);
						if (itemData != nullptr)
						{
							Item::CreateItem(itemData)->PickUp(mando);
							mando->AddItem(itemData);
						}
					}
				}
			}
			if (bootsButton && bootsButton->GetState() == UIButtonState::RELEASED)
			{
				if (mando->beskar > gameManager->bootsLvl && gameManager->bootsLvl + 1 < MAX_ITEM_PRICE)
				{
					gameManager->bootsLvl = bootsSlider->IncrementOneSquare();
					mando->SubtractBeskar(gameManager->bootsLvl);
					bootsPriceText->SetText(std::to_string(gameManager->bootsLvl + 1).c_str());

					if (mando != nullptr)
					{
						ItemData* const itemData = Item::FindItem(hubItems, "Propulsed Boots", (ItemRarity)gameManager->bootsLvl);
						if (itemData != nullptr)
						{
							Item::CreateItem(itemData)->PickUp(mando);
							mando->AddItem(itemData);
						}
					}
				}
			}
			if (ticketButton && ticketButton->GetState() == UIButtonState::RELEASED)
			{
				if (mando->beskar > gameManager->ticketLvl && gameManager->ticketLvl + 1 < MAX_ITEM_PRICE)
				{
					gameManager->ticketLvl = ticketSlider->IncrementOneSquare();
					mando->SubtractBeskar(gameManager->ticketLvl);
					ticketPriceText->SetText(std::to_string(gameManager->ticketLvl + 1).c_str());

					if (mando != nullptr)
					{
						
						ItemData* const itemData = Item::FindItem(hubItems, "Premium Ticket", (ItemRarity)gameManager->ticketLvl);
						if (itemData != nullptr)
						{
							Item::CreateItem(itemData)->PickUp(mando);
							mando->AddItem(itemData);
						}
					}
				}
			}
			if (bottleButton && bottleButton->GetState() == UIButtonState::RELEASED)
			{
				if (mando->beskar > gameManager->bottleLvl && gameManager->bottleLvl + 1 < MAX_ITEM_PRICE)
				{
					gameManager->bottleLvl = bottleSlider->IncrementOneSquare();
					mando->SubtractBeskar(gameManager->bottleLvl);
					bottlePriceText->SetText(std::to_string(gameManager->bottleLvl + 1).c_str());

					if (mando != nullptr)
					{
						ItemData* const itemData = Item::FindItem(hubItems, "Refrigeration Liquid", (ItemRarity)gameManager->bottleLvl);
						if (itemData != nullptr)
						{
							Item::CreateItem(itemData)->PickUp(mando);
							mando->AddItem(itemData);
						}
					}
				}
			}
			if (armorButton != nullptr && armorButton->GetState() == UIButtonState::HOVERED)
			{
				int level = gameManager->armorLvl + 1;
				if (level == MAX_ITEM_PRICE)
					level = MAX_ITEM_PRICE - 1;
				ItemData* const itemData = Item::FindItem(hubItems, "Durasteel Reinforcement", (ItemRarity)level);
				titleText->SetText(itemData->name.c_str());
				descriptionText->SetText(itemData->description.c_str());
			}
			else if (bootsButton != nullptr && bootsButton->GetState() == UIButtonState::HOVERED)
			{
				int level = gameManager->bootsLvl + 1;
				if (level == MAX_ITEM_PRICE)
					level = MAX_ITEM_PRICE - 1;
				ItemData* const itemData = Item::FindItem(hubItems, "Propulsed Boots", (ItemRarity)level);
				titleText->SetText(itemData->name.c_str());
				descriptionText->SetText(itemData->description.c_str());
			}
			else if (ticketButton != nullptr && ticketButton->GetState() == UIButtonState::HOVERED)
			{
				int level = gameManager->armorLvl + 1;
				if (level == MAX_ITEM_PRICE)
					level = MAX_ITEM_PRICE - 1;
				ItemData* const itemData = Item::FindItem(hubItems, "Premium Ticket", (ItemRarity)level);
				titleText->SetText(itemData->name.c_str());
				descriptionText->SetText(itemData->description.c_str());
			}
			else if (bottleButton != nullptr && bottleButton->GetState() == UIButtonState::HOVERED)
			{
				int level = gameManager->bottleLvl + 1;
				if (level == MAX_ITEM_PRICE)
					level = MAX_ITEM_PRICE - 1;
				ItemData* const itemData = Item::FindItem(hubItems, "Refrigeration Liquid", (ItemRarity)level);
				titleText->SetText(itemData->name.c_str());
				descriptionText->SetText(itemData->description.c_str());
			}
		}

		if (beskarText != nullptr)
		{
			std::string tmp = "";
			tmp += std::to_string(mando->beskar).c_str();
			beskarText->SetText(tmp.c_str());
		}
		//if (creditsText != nullptr)
		//{
		//
		//	std::string tmp = "";
		//	tmp += std::to_string(mando->currency).c_str();
		//	creditsText->SetText(tmp.c_str());
		//}
	}
}

HUBArmorer* CreateHUBArmorer() {
	HUBArmorer* script = new HUBArmorer();

	INSPECTOR_STRING(script->mandoName);
	INSPECTOR_STRING(script->hubShopCanvasName);

	INSPECTOR_GAMEOBJECT(script->armorButtonObject);
	INSPECTOR_GAMEOBJECT(script->bootsButtonObject);
	INSPECTOR_GAMEOBJECT(script->ticketButtonObject);
	INSPECTOR_GAMEOBJECT(script->bottleButtonObject);
	INSPECTOR_GAMEOBJECT(script->armorSliderObject);
	INSPECTOR_GAMEOBJECT(script->ticketSliderObject);
	INSPECTOR_GAMEOBJECT(script->bootsSliderObject);
	INSPECTOR_GAMEOBJECT(script->bottleSliderObject);
	INSPECTOR_GAMEOBJECT(script->armorTextObject);
	INSPECTOR_GAMEOBJECT(script->bootsTextObject);
	INSPECTOR_GAMEOBJECT(script->ticketTextObject);
	INSPECTOR_GAMEOBJECT(script->bottleTextObject); 
	INSPECTOR_GAMEOBJECT(script->titleTextObject);
	INSPECTOR_GAMEOBJECT(script->descriptionTextObject);
	//INSPECTOR_GAMEOBJECT(script->creditTextObject);
	INSPECTOR_GAMEOBJECT(script->beskarTextObject);

	return script;
}