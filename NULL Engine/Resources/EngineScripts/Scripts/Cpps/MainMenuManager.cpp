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

MainMenuManager::MainMenuManager() : Script()
{
}

MainMenuManager::~MainMenuManager()
{
}

void MainMenuManager::Start()
{
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
	if (exitButtonObject != nullptr)
		exitButton = (C_UI_Button*)exitButtonObject->GetComponent<C_UI_Button>();

	//settings
	if (settingsCanvasObject)
		settingsCanvas = settingsCanvasObject->GetComponent<C_Canvas>();
	if (fullScreenCheckObject != nullptr)
		fullScreenCheck = (C_UI_Checkbox*)fullScreenCheckObject->GetComponent<C_UI_Checkbox>();
	if (vsyncCheckObject != nullptr)
		vsyncCheck = (C_UI_Checkbox*)vsyncCheckObject->GetComponent<C_UI_Checkbox>();
	if (backButtonObject != nullptr)
		backButton = (C_UI_Button*)backButtonObject->GetComponent<C_UI_Button>();
	if (sliderObject) 
	{
		slider = (C_UI_Slider*)sliderObject->GetComponent<C_UI_Slider>();
		volume = slider->InputValue(volume, 5.0f);
		//TODO: call cool audio function to change the volume????
		//slider->Hoverable(false);
	}
}

void MainMenuManager::Update()
{
	//main
	if (playButton && playButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->ResetArmorerItemsLvl();
		gameManagerScript->GenerateNewRun(true);
		gameManagerScript->InitiateLevel(1);
	}
	if (continueButton && continueButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->Continue();
	}
	if (settingsButton && settingsButton->GetState() == UIButtonState::RELEASED)
	{
		App->uiSystem->PopCanvas();
		App->uiSystem->PushCanvas(settingsCanvas);
		if (fullScreenCheck && App->window->IsFullscreenDesktop())
			fullScreenCheck->SetChecked();
		if(vsyncCheck && App->renderer->GetVsync())
			vsyncCheck->SetChecked();
	}
	if (exitButton && exitButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		App->quit = true;
	}

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
	if (backButton && backButton->GetState() == UIButtonState::RELEASED)
	{
		App->uiSystem->PopCanvas();
		App->uiSystem->PushCanvas(mainCanvas);
	}
	if (slider && slider->Hovered() && (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_DOWN))
		volume = slider->IncrementOneSquare(); //TODO: call cool audio function to change the volume????
	else if (slider && slider->Hovered() && (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_DOWN))
		volume = slider->DecrementOneSquare(); //TODO: call cool audio function to change the volume????
}
