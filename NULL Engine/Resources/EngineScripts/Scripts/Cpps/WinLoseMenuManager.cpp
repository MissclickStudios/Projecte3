#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "GameObject.h"
#include "WinLoseMenuManager.h"

WinLoseMenuManager::WinLoseMenuManager()
{
}

WinLoseMenuManager::~WinLoseMenuManager()
{
}

void WinLoseMenuManager::Start()
{
	backButton = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();
}

void WinLoseMenuManager::Update()
{
	if (backButton != nullptr)
	{
		if (backButton->IsPressed())
		{

		}
	}
}
