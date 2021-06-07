#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "GameObject.h"
#include "WinLoseSceneManager.h"
#include "GameManager.h"
#include "M_UISystem.h"
#include "M_Input.h"

WinLoseSceneManager::WinLoseSceneManager() : Script()
{
}

WinLoseSceneManager::~WinLoseSceneManager()
{
}

void WinLoseSceneManager::Start()
{
	if (canvasObject)
	{
		C_Canvas* canvas = canvasObject->GetComponent<C_Canvas>();
		if (canvas)
			App->uiSystem->PushCanvas(canvas);
	}

}

void WinLoseSceneManager::Update()
{
	if (gameManagerObject && App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
	{
		GameManager* gameManagerScript = (GameManager*)gameManagerObject->GetScript("GameManager");

		if (strcmp(App->scene->GetCurrentScene(), "WinScene") == 0)
			gameManagerScript->GoNextRoom();
		else
			gameManagerScript->ReturnHub();
		
	}
}