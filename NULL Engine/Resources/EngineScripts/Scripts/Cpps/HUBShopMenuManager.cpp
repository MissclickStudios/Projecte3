#include "Application.h"

#include "Log.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "GameObject.h"

#include "GameManager.h"
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


	a = App->scene->GetGameObjectByName("Game Manager");
	if (a != nullptr)
		gameManager = (GameManager*)a->GetScript("GameManager");

	beskarTextHubShop = (C_UI_Text*)gameObject->GetComponent<C_UI_Text>();

	mando = App->scene->GetGameObjectByName(mandoName.c_str());
}

void HUBShopMenuManager::Update()
{
	if (beskarTextHubShop != nullptr)
	{
		if (mando != nullptr)
		{
			Player* playerScript = (Player*)mando->GetScript("Player");

			beskarTextHubShop->SetText(std::to_string(playerScript->beskar).c_str());
		}
	}
}