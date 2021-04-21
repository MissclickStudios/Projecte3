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
	optionsButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName1.c_str())->GetComponent<C_UI_Button>();
	continueButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName2.c_str())->GetComponent<C_UI_Button>();
	exitButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName3.c_str())->GetComponent<C_UI_Button>();
}

void MainMenuManager::Update()
{

	//Continue Button
	if (continueButton != nullptr)
	{
		if (continueButton->IsPressed())
		{

		}
	}

	//Main Menu Button
	if (optionsButton != nullptr)
	{
		if (optionsButton->IsPressed())
		{
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Options Play Button
	if (playButton != nullptr)
	{
		if (playButton->IsPressed())
		{

		}
	}


	//Start Play Button
	if (exitButton != nullptr)
	{
		if (exitButton->IsPressed())
		{

		}
	}
	
}
