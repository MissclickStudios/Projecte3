#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "LevelGenerator.h"
#include "GameObject.h"
#include "HUBShopMenuManager.h"

HUBShopMenuManager::HUBShopMenuManager()
{
}

HUBShopMenuManager::~HUBShopMenuManager()
{
}

void HUBShopMenuManager::Start()
{
	button = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();
}

void HUBShopMenuManager::Update()
{
	if (button != nullptr)
	{
		if (button->IsPressed())
		{

		}
	}
}