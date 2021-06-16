#include "Application.h"
#include "M_Scene.h"
#include "M_Input.h"
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Audio.h"

#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Checkbox.h"
#include "C_UI_Slider.h"
#include "GameObject.h"
#include "M_UISystem.h"
#include "PauseMenuManager.h"
#include "Player.h"
#include "GameManager.h"
#include "HUDManager.h"

PauseMenuManager::PauseMenuManager() : Script()
{
}

PauseMenuManager::~PauseMenuManager()
{
}

void PauseMenuManager::Start()
{
	pauseMenuCanvas = (C_Canvas*)gameObject->GetComponent<C_Canvas>();

	GameObject* go = App->scene->GetGameObjectByName(hudCanvasStr.c_str());
	if (go != nullptr) 
	{
		hudCanvas = go->GetComponent<C_Canvas>();
		hudScript = (HUDManager*)go->GetScript("HUDManager");
	}

	go = App->scene->GetGameObjectByName(resumeButtonName.c_str());
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
	go = App->scene->GetGameObjectByName(musicSliderStr.c_str());
	if (go != nullptr) 
	{
		musicSlider = (C_UI_Slider*)go->GetComponent<C_UI_Slider>();
		if (musicSlider)
		{
			App->audio->maxMusicVolume = musicSlider->InputValue(App->audio->maxMusicVolume, 100.0f, 10);
			App->audio->SetRtcp("maxMusicVolume", App->audio->maxMusicVolume);
		}
	}
	go = App->scene->GetGameObjectByName(fxSliderStr.c_str());
	if (go != nullptr) 
	{
		fxSlider = (C_UI_Slider*)go->GetComponent<C_UI_Slider>();
		if (fxSlider)
		{
			App->audio->maxSfxVolume = fxSlider->InputValue(App->audio->maxSfxVolume, 100.0f, 10);
			App->audio->SetRtcp("maxSfxVolume", App->audio->maxSfxVolume);
		}
	}
	go = App->scene->GetGameObjectByName(optionsMenuCanvasStr.c_str());
	if (go)
		optionsMenuCanvas = (C_Canvas*)go->GetComponent<C_Canvas>();
	go = App->scene->GetGameObjectByName(optionsFullscreenStr.c_str());
	if (go != nullptr)
		fullScreenCheck = (C_UI_Checkbox*)go->GetComponent<C_UI_Checkbox>();
	go = App->scene->GetGameObjectByName(optionsVsyncStr.c_str());
	if (go != nullptr)
		vsyncCheck = (C_UI_Checkbox*)go->GetComponent<C_UI_Checkbox>();
	go = App->scene->GetGameObjectByName(optionsShakeStr.c_str());
	if (go != nullptr)
		shakeCheck = (C_UI_Checkbox*)go->GetComponent<C_UI_Checkbox>();
	go = App->scene->GetGameObjectByName(backButtonStr.c_str());
	if (go != nullptr)
		backButton = (C_UI_Button*)go->GetComponent<C_UI_Button>();

	go = App->scene->GetGameObjectByName(gameManagerName.c_str());
	if (go != nullptr)
		gameManager = (GameManager*)go->GetScript("GameManager");
	mando = App->scene->GetGameObjectByName(mandoName.c_str());
}

void PauseMenuManager::Update()
{
	if (pauseMenuCanvas != nullptr && !onSettings)
	{
		if (gameManager && App->input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(6) == ButtonState::BUTTON_DOWN)
		{
			if (canvasActive)
			{
				App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
				canvasActive = false;
				gameManager->Resume();

				if (hudCanvas && hudScript && hudScript->enabled)
					App->uiSystem->PushCanvas(hudCanvas);

			}
			else 
			{
				if (hudCanvas && hudScript && hudScript->enabled)
					App->uiSystem->RemoveActiveCanvas(hudCanvas);

				App->uiSystem->PushCanvas(pauseMenuCanvas);
				canvasActive = true;
				gameManager->Pause();
			}
		}
	}

	//Continue Button
	if (gameManager && resumeButton != nullptr && pauseMenuCanvas != nullptr)
	{
		if (resumeButton->GetState() == UIButtonState::RELEASED)
		{
			App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
			canvasActive = false;
			gameManager->Resume();

			if (hudCanvas && hudScript && hudScript->enabled)
				App->uiSystem->PushCanvas(hudCanvas);
		}
	}

	//Main Menu Button
	if (gameManager && mainMenuButton != nullptr && gameManager != nullptr)
	{
		if (mainMenuButton->GetState() == UIButtonState::RELEASED)
			gameManager->ReturnToMainMenu();
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
		if (shakeCheck && gameManager->cameraShake)
			shakeCheck->SetChecked();
	}

	//Abandon Run Button
	if (gameManager && abandonRunButton != nullptr && mando != nullptr)
	{
		if (abandonRunButton->GetState() == UIButtonState::RELEASED)
		{
			Player* playerScript = (Player*)mando->GetScript("Player");
			playerScript->health = 0;
			gameManager->Resume();

			App->uiSystem->RemoveActiveCanvas(pauseMenuCanvas);
		}
	}

	//settings
	if (musicSlider && musicSlider->Hovered() && (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_DOWN))
	{
		App->audio->maxMusicVolume = musicSlider->IncrementOneSquare();
		App->audio->SetRtcp("maxMusicVolume", App->audio->maxMusicVolume);
	}
	else if (musicSlider && musicSlider->Hovered() && (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_DOWN))
	{
		App->audio->maxMusicVolume = musicSlider->DecrementOneSquare();
		App->audio->SetRtcp("maxMusicVolume", App->audio->maxMusicVolume);
	}
	if (fxSlider && fxSlider->Hovered() && (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_DOWN))
	{
		App->audio->maxSfxVolume = fxSlider->IncrementOneSquare();
		App->audio->SetRtcp("maxSfxVolume", App->audio->maxSfxVolume);
	}
	else if (fxSlider && fxSlider->Hovered() && (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_DOWN))
	{
		App->audio->maxSfxVolume = fxSlider->DecrementOneSquare();
		App->audio->SetRtcp("maxSfxVolume", App->audio->maxSfxVolume);
	}
	if (fullScreenCheck)
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
	if (gameManager && shakeCheck)
	{
		if (shakeCheck->GetState() == UICheckboxState::PRESSED_UNCHECKED_OUT)
			gameManager->cameraShake = true;
		else if (shakeCheck->GetState() == UICheckboxState::PRESSED_CHECKED_OUT)
			gameManager->cameraShake = false;
	}
	if (pauseMenuCanvas && optionsMenuCanvas && backButton && backButton->GetState() == UIButtonState::RELEASED) 
	{
		App->uiSystem->RemoveActiveCanvas(optionsMenuCanvas);
		App->uiSystem->PushCanvas(pauseMenuCanvas);
		onSettings = false;
	}
}