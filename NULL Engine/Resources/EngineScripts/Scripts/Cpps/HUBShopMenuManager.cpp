#include "Application.h"

#include "Log.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "GameObject.h"

#include "Player.h"
#include "HUBShopMenuManager.h"

HUBShopMenuManager::HUBShopMenuManager()
{
}

HUBShopMenuManager::~HUBShopMenuManager()
{
}

void HUBShopMenuManager::Start()
{
	GameObject* a = App->scene->GetGameObjectByName(exitButtonName.c_str());
	//if(a != nullptr)
		//exitButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	a = App->scene->GetGameObjectByName(buyButtonName.c_str());
	if (a != nullptr)
		buyButton = (C_UI_Button*)a->GetComponent<C_UI_Button>();

	beskarTextHubShop = (C_UI_Text*)gameObject->GetComponent<C_UI_Text>();

	mando = App->scene->GetGameObjectByName(mandoName.c_str());
}

void HUBShopMenuManager::Update()
{
	/*if (exitButton != nullptr)
	{
		if (exitButton->GetState() == UIButtonState::RELEASED)
		{
			exitButton->SetIsPressed(false);
			exitButton->SetState(UIButtonState::HOVERED);
			gameObject->GetComponent<C_Canvas>()->SetIsActive(false);
		}
	}*/

	if (buyButton != nullptr)
	{
		if (buyButton->GetState() == UIButtonState::RELEASED)
		{
			if (mando != nullptr)
			{
				Player* playerScript = (Player*)mando->GetScript("Player");

				if (playerScript->hubCurrency - beskarCost >= 0)
				{
					playerScript->hubCurrency -= beskarCost;
				}
			}
		}
	}

	if (beskarTextHubShop != nullptr)
	{
		if (mando != nullptr)
		{
			Player* playerScript = (Player*)mando->GetScript("Player");

			beskarTextHubShop->SetText(std::to_string(playerScript->hubCurrency).c_str());
		}
	}
}