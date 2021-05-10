#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "GameObject.h"
#include "M_UISystem.h"
#include "PauseMenuManager.h"
#include "Player.h"
#include "GameManager.h"

PauseMenuManager::PauseMenuManager() : Script()
{
}

PauseMenuManager::~PauseMenuManager()
{
}

void PauseMenuManager::Start()
{
	pauseMenuCanvas = (C_Canvas*)gameObject->GetComponent<C_Canvas>();

	GameObject* a = App->scene->GetGameObjectByName(resumeButtonName.c_str());
	if (a != nullptr)
		resumeButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(abandonRunButtonName.c_str());
	if (a != nullptr)
		abandonRunButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(mainMenuButtonName.c_str());
	if (a != nullptr)
		mainMenuButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(exitButtonName.c_str());
	if (a != nullptr)
		exitButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	mando = App->scene->GetGameObjectByName(mandoName.c_str());
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());

	//optionsButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName1.c_str())->GetComponent<C_UI_Button>();
}

void PauseMenuManager::Update()
{
	if (pauseMenuCanvas != nullptr)
	{
		if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(6) == ButtonState::BUTTON_DOWN)
		{
			if (canvasActive)
			{
				App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
				canvasActive = false;
			}
			else 
			{
				App->uiSystem->PushCanvas(pauseMenuCanvas);
				canvasActive = true;
			}
		}
	}

	//Continue Button
	if (resumeButton != nullptr && pauseMenuCanvas != nullptr)
	{
		if (resumeButton->GetState() == UIButtonState::RELEASED)
		{
			App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
			canvasActive = false;
		}
	}

	//Main Menu Button
	if (mainMenuButton != nullptr && gameManager != nullptr)
	{
		if (mainMenuButton->GetState() == UIButtonState::RELEASED)
		{
			GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
			gameManagerScript->ReturnToMainMenu();
		}
	}

	//Options Play Button
	if (optionsButton != nullptr)
	{
		if (optionsButton->GetState() == UIButtonState::RELEASED)
		{

		}
	}

	//Abandon Run Button
	if (abandonRunButton != nullptr && mando != nullptr)
	{
		if (abandonRunButton->GetState() == UIButtonState::RELEASED)
		{
			Player* playerScript = (Player*)mando->GetScript("Player");
			playerScript->health = 0;

			App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
		}
	}

	//Start Play Button
	if (exitButton != nullptr)
	{
		if (exitButton->GetState() == UIButtonState::RELEASED)
			App->quit = true; //wtf is this shit
	}

}