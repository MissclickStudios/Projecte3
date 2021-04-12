#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"

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
	if (playButton != nullptr)
	{
		if (playButton->IsPressed())
		{
			App->scene->GetLevelGenerator().InitiateLevel(1);
		}
	}
	
}
