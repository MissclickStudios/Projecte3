#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "GameObject.h"
#include "MainMenuManager.h"
#include "GameManager.h"
#include "M_UISystem.h"

MainMenuManager::MainMenuManager() : Script()
{
}

MainMenuManager::~MainMenuManager()
{
}

void MainMenuManager::Start()
{
	if (canvasObject) 
	{
		C_Canvas* canvas = canvasObject->GetComponent<C_Canvas>();
		if (canvas)
			App->uiSystem->PushCanvas(canvas);
	}
	if(playButtonObject != nullptr)
		playButton = (C_UI_Button*)playButtonObject->GetComponent<C_UI_Button>();
	if (continueButtonObject != nullptr)
		continueButton = (C_UI_Button*)continueButtonObject->GetComponent<C_UI_Button>();
	if (exitButtonObject != nullptr)
		exitButton = (C_UI_Button*)exitButtonObject->GetComponent<C_UI_Button>();
}

void MainMenuManager::Update()
{
	if (playButton && playButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->GenerateNewRun(true);
		gameManagerScript->InitiateLevel(1);
	}
	if (continueButton && continueButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->Continue();
	}
	if (exitButton && exitButton->GetState() == UIButtonState::RELEASED && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		App->quit = true;
	}
}
