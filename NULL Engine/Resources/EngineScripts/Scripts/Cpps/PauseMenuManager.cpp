#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"
#include "M_Window.h"
#include "M_Renderer3D.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Checkbox.h"
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

	GameObject* go = App->scene->GetGameObjectByName(resumeButtonName.c_str());
	if (go != nullptr)
		resumeButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();
	
	go = App->scene->GetGameObjectByName(optionsButtonName.c_str());
	if (go != nullptr)
		optionsButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();

	go = App->scene->GetGameObjectByName(abandonRunButtonName.c_str());
	if (go != nullptr)
		abandonRunButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();

	go = App->scene->GetGameObjectByName(mainMenuButtonName.c_str());
	if (go != nullptr)
		mainMenuButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();

	//settings
	go = App->scene->GetGameObjectByName(optionsMenuCanvasStr.c_str());
	if (go)
		optionsMenuCanvas = (C_Canvas*)go->GetComponent<C_Canvas>();
	go = App->scene->GetGameObjectByName(optionsFullscreenStr.c_str());
	if (go != nullptr)
		fullScreenCheck = (C_UI_Checkbox*)go->GetComponent<C_UI_Checkbox>();
	go = App->scene->GetGameObjectByName(optionsVsyncStr.c_str());
	if (go != nullptr)
		vsyncCheck = (C_UI_Checkbox*)go->GetComponent<C_UI_Checkbox>();
	go = App->scene->GetGameObjectByName(backButtonStr.c_str());
	if (go != nullptr)
		backButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();

	mando = App->scene->GetGameObjectByName(mandoName.c_str());
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
}

void PauseMenuManager::Update()
{
	if (pauseMenuCanvas != nullptr && !onSettings)
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

	//Settings Button
	if (pauseMenuCanvas && optionsMenuCanvas && optionsButton && optionsButton->GetState() == UIButtonState::RELEASED)
	{
		onSettings = true;
		App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
		App->uiSystem->PushCanvas(optionsMenuCanvas);
		if (fullScreenCheck && App->window->IsFullscreenDesktop())
			fullScreenCheck->SetChecked();
		if (vsyncCheck && App->renderer->GetVsync())
			vsyncCheck->SetChecked();
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

	//settings
	if (fullScreenCheck != nullptr)
	{
		if (fullScreenCheck->GetState() == UICheckboxState::PRESSED_UNCHECKED_OUT)
			App->window->SetFullscreenDesktop(true);
		else if (fullScreenCheck->GetState() == UICheckboxState::PRESSED_CHECKED_OUT)
			App->window->SetFullscreenDesktop(false);
	}
	if (vsyncCheck)
	{
		if (vsyncCheck->GetState() == UICheckboxState::PRESSED_UNCHECKED_OUT)
			App->renderer->SetVsync(true);
		else if (vsyncCheck->GetState() == UICheckboxState::PRESSED_CHECKED_OUT)
			App->renderer->SetVsync(false);
	}
	if (pauseMenuCanvas && optionsMenuCanvas && backButton && backButton->GetState() == UIButtonState::RELEASED) 
	{
		App->uiSystem->RemoveActiveCanvas(optionsMenuCanvas);
		App->uiSystem->PushCanvas(pauseMenuCanvas);
		onSettings = false;
	}
}