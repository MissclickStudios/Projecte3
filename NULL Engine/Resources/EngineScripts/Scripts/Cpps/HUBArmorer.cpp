#include "Application.h"
#include "M_Scene.h"
#include "Log.h"

#include "M_Input.h"

#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"

#include "GameObject.h"
#include "HUBArmorer.h"

HUBArmorer::HUBArmorer()
{
}

HUBArmorer::~HUBArmorer()
{
}

void HUBArmorer::Start()
{
	mando = App->scene->GetGameObjectByName(mandoName.c_str());
	hubShopCanvas = App->scene->GetGameObjectByName(hubShopCanvasName.c_str());
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
				//if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_UP )
				//{
				App->uiSystem->PushCanvas(hubShopCanvas);
				//}
				state = HUBArmorerState::ACTIVE;
			}
			break;
		case HUBArmorerState::ACTIVE:
			if (mando->transform->GetDistanceTo(gameObject->transform->GetLocalPosition()) >= talkDistance)
			{
				//if (App->input->GetKey(SDL_SCANCODE_B) == KeyState::KEY_UP || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_UP )
				//{
				App->uiSystem->RemoveActiveCanvas(hubShopCanvas);
				//}
				state = HUBArmorerState::INACTIVE;
			}
			break;
		default:
			state = HUBArmorerState::INACTIVE; break;
		}
	}
}