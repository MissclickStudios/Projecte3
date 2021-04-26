#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "GameObject.h"
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
	GameObject* a = App->scene->GetGameObjectByName(pauseMenuCanvasName.c_str());
	if(a != nullptr)
		pauseMenuCanvas = (C_Canvas*)a->GetComponent<C_Canvas>();

	a = App->scene->GetGameObjectByName(continueButtonName.c_str());
	if (a != nullptr)
		continueButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

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
			if (pauseMenuCanvas->IsActive())
				pauseMenuCanvas->SetIsActive(false);
			else
				pauseMenuCanvas->SetIsActive(true);
		}
	}

	//Continue Button
	if (continueButton != nullptr)
	{
		if (continueButton->GetState() == UIButtonState::RELEASED)
		{
			if (pauseMenuCanvas != nullptr)
			{
				pauseMenuCanvas->SetIsActive(false);
			}
		}
	}

	//Main Menu Button
	if (mainMenuButton != nullptr)
	{
		if (mainMenuButton->GetState() == UIButtonState::RELEASED)
		{
			if (gameManager != nullptr)
			{
				GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
				gameManagerScript->ReturnToMainMenu();
			}
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
	if (abandonRunButton != nullptr)
	{
		if (abandonRunButton->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* playerScript = (Player*)mando->GetScript("Player");
				playerScript->health = 0;

				pauseMenuCanvas->SetIsActive(false);
			}
		}
	}

	//Start Play Button
	if (exitButton != nullptr)
	{
		if (exitButton->GetState() == UIButtonState::RELEASED)
		{
			App->quit = true; //wtf is this shit
		}
	}

}