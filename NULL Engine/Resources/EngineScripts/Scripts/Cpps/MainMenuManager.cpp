#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "MainMenuManager.h"

MainMenuManager::MainMenuManager() : Script()
{
}

MainMenuManager::~MainMenuManager()
{
}

void MainMenuManager::Start()
{
	playButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();

}

void MainMenuManager::Update()
{

	//Start Play Button
	if (playButton != nullptr)
	{
		if (playButton->IsPressed())
		{
			App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Continue Button
	if (continueButton != nullptr)
	{
		if (continueButton->IsPressed())
		{
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Start Play Button
	if (optionsButton != nullptr)
	{
		if (optionsButton->IsPressed())
		{
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Start Play Button
	if (exitButton != nullptr)
	{
		if (exitButton->IsPressed())
		{
			SDL_Quit();
		}
	}
	
}
