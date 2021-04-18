#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "MainMenuManager.h"
#include "GameManager.h"

MainMenuManager::MainMenuManager() : Script()
{
}

MainMenuManager::~MainMenuManager()
{
}

void MainMenuManager::Start()
{
	if(newGameButton != nullptr)
		playButton = (C_UI_Button*)newGameButton->GetComponent<C_UI_Button>();
}

void MainMenuManager::Update()
{
	if (playButton->IsPressed() && gameManager != nullptr)
	{
		GameManager* gameManagerScript = (GameManager*)gameManager->GetScript("GameManager");
		gameManagerScript->GenerateNewRun();
		gameManagerScript->level.InitiateLevel(1);
	}
}
