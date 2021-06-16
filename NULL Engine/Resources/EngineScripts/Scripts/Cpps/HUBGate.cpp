#include "Application.h"
#include "GameManager.h"
#include "Player.h"
#include "GameObject.h"
#include "M_UISystem.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_Transform.h"
#include "M_Scene.h"

#include "HUBGate.h"

HUBGate::HUBGate() : Script()
{
}

HUBGate::~HUBGate()
{
}

void HUBGate::Start()
{
	if (gameManagerObject)
		gameManager = (GameManager*)gameManagerObject->GetScript("GameManager");
	GameObject* playerObject = App->scene->GetGameObjectByName(playerStr.c_str());
	if (playerObject)
		player = (Player*)playerObject->GetScript("Player");
	if (popUpCanvasObject)
		popUpCanvas = popUpCanvasObject->GetComponent<C_Canvas>();
	if (yesButtonObject)
		yesButton = yesButtonObject->GetComponent<C_UI_Button>();
	//if (noButtonObject)
	//	noButton = noButtonObject->GetComponent<C_UI_Button>();
}

void HUBGate::Update()
{
	if (gameManager && player && triggered && yesButton /*&& noButton*/)
	{
		if (yesButton->GetState() == UIButtonState::RELEASED)
		{
			gameManager->GoNextRoom();
			App->uiSystem->RemoveActiveCanvas(popUpCanvas);
		}
		//else if (noButton->GetState() == UIButtonState::RELEASED)
		//{
		//	//TODO: apartar el player del collider pk no es faci insta trigger ???
		//	triggered = false;
		//	App->uiSystem->RemoveActiveCanvas(popUpCanvas);
		//}

		//TODO: ns si es podra moure el player durant el popup
		if (player->transform->GetDistanceTo(gameObject->transform->GetLocalPosition()) >= 10.0f) 
		{
			triggered = false;
			App->uiSystem->RemoveActiveCanvas(popUpCanvas);
		}
	}
	
}

void HUBGate::OnCollisionEnter(GameObject* object)
{
	if (!triggered) 
	{
		App->uiSystem->PushCanvas(popUpCanvas);
		triggered = true;
	}
}


HUBGate* CreateHUBGate()
{
	HUBGate* script = new HUBGate();
	INSPECTOR_GAMEOBJECT(script->gameManagerObject);
	INSPECTOR_GAMEOBJECT(script->popUpCanvasObject);
	INSPECTOR_GAMEOBJECT(script->yesButtonObject);
	//INSPECTOR_GAMEOBJECT(script->noButtonObject);
	INSPECTOR_STRING(script->playerStr);
	return script;
}