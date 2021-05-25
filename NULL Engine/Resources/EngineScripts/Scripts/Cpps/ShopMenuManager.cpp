#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "GameObject.h"
#include "ShopMenuManager.h"

#include "GroundItem.h"

ShopMenuManager::ShopMenuManager()
{
}

ShopMenuManager::~ShopMenuManager()
{
}

void ShopMenuManager::Start()
{
	button = (C_UI_Button*)App->scene->GetGameObjectByName(buttonName.c_str())->GetComponent<C_UI_Button>();
}

void ShopMenuManager::Update()
{
	if (button && button->GetState() == UIButtonState::PRESSEDIN)
	{

	}
}