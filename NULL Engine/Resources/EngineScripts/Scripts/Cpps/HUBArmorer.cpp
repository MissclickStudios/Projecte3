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

#define MAX_ITEM_LEVEL 3

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
			//armorSlider->InputValue();
		}
	}
	if (ticketSliderObject != nullptr) 
	{
		ticketSlider = (C_UI_Slider*)ticketSliderObject->GetComponent<C_UI_Slider>();
		if (ticketSlider)
		{
			ticketSlider->Hoverable(false);
			//ticketSlider->InputValue();
		}
	}
	if (creditsSliderObject != nullptr)
	{
		creditsSlider = (C_UI_Slider*)creditsSliderObject->GetComponent<C_UI_Slider>();
		if (creditsSlider)
		{
			creditsSlider->Hoverable(false);
			//creditsSlider->InputValue();
		}
	}
	if (bottleSliderObject != nullptr)
	{
		bottleSlider = (C_UI_Slider*)bottleSliderObject->GetComponent<C_UI_Slider>();
		if (bottleSlider)
		{
			bottleSlider->Hoverable(false);
			//bottleSlider->InputValue();
		}
	}
	if (armorTextObject != nullptr)
		armortext = (C_UI_Text*)armorTextObject->GetComponent<C_UI_Text>();
	if (bootsTextObject != nullptr)
		bootstext = (C_UI_Text*)bootsTextObject->GetComponent<C_UI_Text>();
	if (ticketTextObject != nullptr)
		tickettext = (C_UI_Text*)ticketTextObject->GetComponent<C_UI_Text>();
	if (bottleTextObject != nullptr)
		bottletext = (C_UI_Text*)bottleTextObject->GetComponent<C_UI_Text>();
	if (descriptionTextObject != nullptr)
		descriptionText = (C_UI_Text*)descriptionTextObject->GetComponent<C_UI_Text>();

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
	INSPECTOR_GAMEOBJECT(script->creditsSliderObject);
	INSPECTOR_GAMEOBJECT(script->bottleSliderObject);
	INSPECTOR_GAMEOBJECT(script->armorTextObject);
	INSPECTOR_GAMEOBJECT(script->bootsTextObject);
	INSPECTOR_GAMEOBJECT(script->ticketTextObject);
	INSPECTOR_GAMEOBJECT(script->bottleTextObject);
	INSPECTOR_GAMEOBJECT(script->descriptionTextObject);

	return script;
}