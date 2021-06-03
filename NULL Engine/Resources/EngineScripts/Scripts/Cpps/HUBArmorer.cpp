#include "Application.h"
#include "M_Scene.h"

#include "M_Input.h"
#include "M_UISystem.h"

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
		armorPriceText = (C_UI_Text*)armorTextObject->GetComponent<C_UI_Text>();
	if (bootsTextObject != nullptr)
		bootsPriceText = (C_UI_Text*)bootsTextObject->GetComponent<C_UI_Text>();
	if (ticketTextObject != nullptr)
		ticketPriceText = (C_UI_Text*)ticketTextObject->GetComponent<C_UI_Text>();
	if (bottleTextObject != nullptr)
		bottlePriceText = (C_UI_Text*)bottleTextObject->GetComponent<C_UI_Text>();
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
		case HUBArmorerState::INACTIVE:
			if (mando->transform->GetDistanceTo(gameObject->transform->GetLocalPosition()) <= talkDistance)
			{
				if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP ) // A/X button on controller
				{
					App->uiSystem->PushCanvas(hubShopCanvas);
					menuOpen = true;
					state = HUBArmorerState::ACTIVE;
					gameManager->TalkedToArmorer();
				}
				
			}
			break;
		case HUBArmorerState::ACTIVE:
			if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_UP) // B/Square button on controller
			{
				App->uiSystem->RemoveActiveCanvas(hubShopCanvas);
				mando->SetPlayerInteraction(InteractionType::NONE);
				menuOpen = false;
				state = HUBArmorerState::INACTIVE;
				break;
			}
			if (mando) {
				if (gameManager)
				{
					if (armorButton && armorButton->GetState() == UIButtonState::RELEASED)
					{
						if (mando->hubCurrency > gameManager->armorLvl && gameManager->armorLvl + 1 < MAX_ITEM_PRICE)
						{
							gameManager->armorLvl = armorSlider->IncrementOneSquare();
							mando->hubCurrency -= gameManager->armorLvl + 1;
							armorPriceText->SetText(std::to_string(gameManager->armorLvl + 1).c_str());
							
							if (mando != nullptr)
							{
								std::vector<ItemData*> hubItems = gameManager->GetHubItemPool();
								ItemData* const itemData = Item::FindItem(hubItems, "Durasteel Reinforcement", (ItemRarity)gameManager->armorLvl);
								if (itemData != nullptr)
									mando->AddItem(itemData);
							}
						}
					}
					if (bootsButton && bootsButton->GetState() == UIButtonState::RELEASED)
					{
						if (mando->hubCurrency > gameManager->bootsLvl && gameManager->bootsLvl + 1 < MAX_ITEM_PRICE)
						{
							gameManager->bootsLvl = bootsSlider->IncrementOneSquare();
							mando->hubCurrency -= gameManager->bootsLvl + 1;
							bootsPriceText->SetText(std::to_string(gameManager->bootsLvl + 1).c_str());

							if (mando != nullptr)
							{
								std::vector<ItemData*> hubItems = gameManager->GetHubItemPool();
								ItemData* const itemData = Item::FindItem(hubItems, "Propulsed Boots", (ItemRarity)gameManager->bootsLvl);
								if (itemData != nullptr)
									mando->AddItem(itemData);
							}
						}
					}
					if (ticketButton && ticketButton->GetState() == UIButtonState::RELEASED)
					{
						if (mando->hubCurrency > gameManager->ticketLvl && gameManager->ticketLvl + 1 < MAX_ITEM_PRICE)
						{
							gameManager->ticketLvl = ticketSlider->IncrementOneSquare();
							mando->hubCurrency -= gameManager->ticketLvl + 1;
							ticketPriceText->SetText(std::to_string(gameManager->ticketLvl + 1).c_str());

							if (mando != nullptr)
							{
								std::vector<ItemData*> hubItems = gameManager->GetHubItemPool();
								ItemData* const itemData = Item::FindItem(hubItems, "Premium Ticket", (ItemRarity)gameManager->ticketLvl);
								if (itemData != nullptr)
									mando->AddItem(itemData);
							}
						}
					}
					if (bottleButton && bottleButton->GetState() == UIButtonState::RELEASED)
					{
						if (mando->hubCurrency > gameManager->bottleLvl && gameManager->bottleLvl + 1 < MAX_ITEM_PRICE)
						{
							gameManager->bottleLvl = bottleSlider->IncrementOneSquare();
							mando->hubCurrency -= gameManager->bottleLvl + 1;
							bottlePriceText->SetText(std::to_string(gameManager->bottleLvl + 1).c_str());

							if (mando != nullptr)
							{
								std::vector<ItemData*> hubItems = gameManager->GetHubItemPool();
								ItemData* const itemData = Item::FindItem(hubItems, "Refrigeration Liquid", (ItemRarity)gameManager->bottleLvl);
								if (itemData != nullptr)
									mando->AddItem(itemData);
							}
						}
					}
				}

				if (beskarText != nullptr)
				{
					std::string tmp = "";
					tmp += std::to_string(mando->hubCurrency).c_str();
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
			break;
		default:
			state = HUBArmorerState::INACTIVE; break;
		}
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
	INSPECTOR_GAMEOBJECT(script->descriptionTextObject);
	//INSPECTOR_GAMEOBJECT(script->creditTextObject);
	INSPECTOR_GAMEOBJECT(script->beskarTextObject);

	return script;
}