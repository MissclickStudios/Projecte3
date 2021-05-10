#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "GameObject.h"
#include "WinSceneManager.h"
#include "GameManager.h"
#include "M_UISystem.h"
#include "M_Input.h"

WinSceneManager::WinSceneManager() : Script()
{
}

WinSceneManager::~WinSceneManager()
{
}

void WinSceneManager::Start()
{
	if (canvasObject)
	{
		C_Canvas* canvas = canvasObject->GetComponent<C_Canvas>();
		if (canvas)
			App->uiSystem->PushCanvas(canvas);
	}

}

void WinSceneManager::Update()
{
	if (gameManagerObject && App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
	{
		GameManager* gameManagerScript = (GameManager*)gameManagerObject->GetScript("GameManager");
		gameManagerScript->GenerateNewRun(false);
		gameManagerScript->InitiateLevel(1);
	}
}