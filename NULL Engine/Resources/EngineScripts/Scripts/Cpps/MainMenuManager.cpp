#include "Application.h"
#include "M_Scene.h"
//#include "C_UI_Button.h"

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
	playButton = (C_UI_Button*)App->scene->GetGameObjectByName("")->GetComponent<C_UI_Button>();

}

void MainMenuManager::Update()
{
	//if (playButton->IsPressed())
	//{
	//	//App->scene->GetLevel()->InitiateLevel(1);
	//}
}
