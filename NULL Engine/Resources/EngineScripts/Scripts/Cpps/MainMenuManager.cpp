#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "PauseMenuManager.h"

PauseMenuManager::PauseMenuManager() : Script()
{
}

PauseMenuManager::~PauseMenuManager()
{
}

void PauseMenuManager::Start()
{
	/*
	continueButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();
	optionsButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName1.c_str())->GetComponent<C_UI_Button>();
	abandonRunButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName2.c_str())->GetComponent<C_UI_Button>();
	mainMenuButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName3.c_str())->GetComponent<C_UI_Button>();
	exitButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName4.c_str())->GetComponent<C_UI_Button>();
	*/
}

void PauseMenuManager::Update()
{

	//Continue Button
	if (continueButton != nullptr)
	{
		if (continueButton->IsPressed())
		{

		}
	}

	//Main Menu Button
	if (mainMenuButton != nullptr)
	{
		if (mainMenuButton->IsPressed())
		{
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Options Play Button
	if (optionsButton != nullptr)
	{
		if (optionsButton->IsPressed())
		{
			//App->scene->GetLevelGenerator()->InitiateLevel(1);
		}
	}

	//Abandon Run Button
	if (abandonRunButton != nullptr)
	{
		if (abandonRunButton->IsPressed())
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
