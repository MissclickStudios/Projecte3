#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "OptionsMenuManager.h"

OptionsMenuManager::OptionsMenuManager() : Script()
{
}

OptionsMenuManager::~OptionsMenuManager()
{
}

void OptionsMenuManager::Start()
{
	backButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();

}

void OptionsMenuManager::Update()
{
	if (backButton != nullptr)
	{
		if (backButton->IsPressed())
		{

		}
	}

}
