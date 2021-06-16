#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Checkbox.h"
#include "C_UI_Slider.h"
#include "GameObject.h"
#include "MainMenuManager.h"
#include "GameManager.h"
#include "M_UISystem.h"
#include "M_Renderer3D.h"
#include "M_Window.h"
#include "M_Input.h"
#include "M_Audio.h"

MainMenuManager::MainMenuManager() : Script()
{
}

MainMenuManager::~MainMenuManager()
{
}

void MainMenuManager::Start()
{
	gameManager = (GameManager*)gameManagerObject->GetScript("GameManager");
	//main
	if (mainCanvasObject)
	{
		mainCanvas = mainCanvasObject->GetComponent<C_Canvas>();
		if (mainCanvas)
			App->uiSystem->PushCanvas(mainCanvas);
	}
	if(playButtonObject != nullptr)
		playButton = (C_UI_Button*)playButtonObject->GetComponent<C_UI_Button>();
	if (continueButtonObject != nullptr)
		continueButton = (C_UI_Button*)continueButtonObject->GetComponent<C_UI_Button>();
	if (settingsButtonObject != nullptr)
		settingsButton = (C_UI_Button*)settingsButtonObject->GetComponent<C_UI_Button>();
	
	creditsButton = (C_UI_Button*)App->scene->GetGameObjectByName("Credits")->GetComponent<C_UI_Button>();

	if (exitButtonObject != nullptr)
		exitButton = (C_UI_Button*)exitButtonObject->GetComponent<C_UI_Button>();

	//settings
	if (settingsCanvasObject)
		settingsCanvas = settingsCanvasObject->GetComponent<C_Canvas>();
	if (fullScreenCheckObject != nullptr)
		fullScreenCheck = (C_UI_Checkbox*)fullScreenCheckObject->GetComponent<C_UI_Checkbox>();
	if (vsyncCheckObject != nullptr)
		vsyncCheck = (C_UI_Checkbox*)vsyncCheckObject->GetComponent<C_UI_Checkbox>();
	if (shakeCheckObject != nullptr)
		shakeCheck = (C_UI_Checkbox*)shakeCheckObject->GetComponent<C_UI_Checkbox>();
	if (backButtonObject != nullptr)
		backButton = (C_UI_Button*)backButtonObject->GetComponent<C_UI_Button>();
	if (musicSliderObject) 
	{
		musicSlider = (C_UI_Slider*)musicSliderObject->GetComponent<C_UI_Slider>();
		if (musicSlider)
		{
			App->audio->maxMusicVolume = musicSlider->InputValue(App->audio->maxMusicVolume, 100.0f, 10);
			App->audio->SetRtcp("maxMusicVolume", App->audio->maxMusicVolume);
		}
	}
	if (fxSliderObject)
	{
		fxSlider = (C_UI_Slider*)fxSliderObject->GetComponent<C_UI_Slider>();
		if (fxSlider)
		{
			App->audio->maxSfxVolume = fxSlider->InputValue(App->audio->maxSfxVolume, 100.0f, 10);
			App->audio->SetRtcp("maxSfxVolume", App->audio->maxSfxVolume);
		}
	}
}

void MainMenuManager::Update()
{
	//main
	if (gameManager && playButton && playButton->GetState() == UIButtonState::RELEASED && gameManagerObject != nullptr)
	{
		gameManager->ResetArmorerItemsLvl();
		gameManager->GenerateNewRun(true);
		gameManager->InitiateLevel(1);
	}
	if (gameManager && continueButton && continueButton->GetState() == UIButtonState::RELEASED && gameManagerObject != nullptr)
		gameManager->Continue();

	if (gameManager && settingsButton && settingsButton->GetState() == UIButtonState::RELEASED)
	{
		App->uiSystem->PopCanvas();
		App->uiSystem->PushCanvas(settingsCanvas);
		if (fullScreenCheck && App->window->IsFullscreenDesktop())
			fullScreenCheck->SetChecked();
		if(vsyncCheck && App->renderer->GetVsync())
			vsyncCheck->SetChecked();
		if (shakeCheck && gameManager->cameraShake)
			shakeCheck->SetChecked();
	}

	if (gameManager && creditsButton && creditsButton->GetState() == UIButtonState::RELEASED)
	{
		App->scene->ScriptChangeScene("Assets/Scenes/Credits.json");
		App->scene->creditsMainMenu = true;
	}

	if (gameManager && exitButton && exitButton->GetState() == UIButtonState::RELEASED && gameManagerObject != nullptr)
		App->quit = true;

	//settings
	if (fullScreenCheck)
	{
		if(fullScreenCheck->GetState() == UICheckboxState::PRESSED_UNCHECKED_OUT)
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
	if (backButton && backButton->GetState() == UIButtonState::RELEASED)
	{
		App->uiSystem->PopCanvas();
		App->uiSystem->PushCanvas(mainCanvas);
	}
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
}
