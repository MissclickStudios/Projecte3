#include "Application.h"

#include "Log.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
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
	exitButton = (C_UI_Button*)App->scene->GetGameObjectByName(exitButtonName.c_str())->GetComponent<C_UI_Button>();
	buyButton = (C_UI_Button*)App->scene->GetGameObjectByName(buyButtonName.c_str())->GetComponent<C_UI_Button>();
}

void HUBShopMenuManager::Update()
{
	if (exitButton != nullptr)
	{
		if (exitButton->IsPressed())
		{
			exitButton->SetIsPressed(false);
			exitButton->SetState(UIButtonState::HOVERED);
			gameObject->GetComponent<C_Canvas>()->SetIsActive(false);
		}
	}

	if (buyButton != nullptr)
	{
		if (buyButton->IsPressed())
		{
			//Use Beskar Ingots
			LOG("OMG YOU HAVE JUST SPENT %d Beskar Ingots!", beskarCost);
		}
	}
}