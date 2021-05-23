#include "Application.h"
#include "M_Scene.h"

#include "M_Input.h"
#include "M_UISystem.h"

#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"

#include "GameManager.h"
#include "GameObject.h"
#include "Player.h"
#include "HUBArmorer.h"

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
				if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_UP ) // A/X button on controller
				{
					App->uiSystem->PushCanvas(hubShopCanvas);
					state = HUBArmorerState::ACTIVE;
					gameManager->TalkedToArmorer();
				}
				
			}
			break;
		case HUBArmorerState::ACTIVE:
			if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_UP) // B/Square button on controller
			{
				App->uiSystem->RemoveActiveCanvas(hubShopCanvas);
				state = HUBArmorerState::INACTIVE;
			}
			break;
		default:
			state = HUBArmorerState::INACTIVE; break;
		}
	}
}