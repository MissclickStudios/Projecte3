#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"
#include "C_2DAnimator.h"
#include "GameObject.h"
#include "HUDManager.h"
#include "Player.h"
#include "M_Input.h"


HUDManager::HUDManager() : Script()
{
}

HUDManager::~HUDManager()
{
}

void HUDManager::Start()
{
	GameObject* a = App->scene->GetGameObjectByName(mandoImageName.c_str());
	if(a != nullptr)
		mandoImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(primaryWeaponImageName.c_str());
	if (a != nullptr)
		primaryWeaponImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(secondaryWeaponImageName.c_str());
	if (a != nullptr)
		secondaryWeaponImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(dashImageName.c_str());
	if (a != nullptr)
		dashImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(creditsImageName.c_str());
	if (a != nullptr)
		creditsImage = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(debugMenuCanvasName.c_str());
	if (a != nullptr)
		debugMenuCanvas = (C_Canvas*)a->GetComponent<C_Canvas>();

	a = App->scene->GetGameObjectByName(hubShopCanvasName.c_str());
	if (a != nullptr)
		hubShopCanvas = (C_Canvas*)a->GetComponent<C_Canvas>();

	a = App->scene->GetGameObjectByName(hudCanvasName.c_str());
	if (a != nullptr)
		hudCanvas = (C_Canvas*)a->GetComponent<C_Canvas>();

	a = App->scene->GetGameObjectByName(pauseMenuCanvasName.c_str());
	if (a != nullptr)
		pauseMenuCanvas = (C_Canvas*)a->GetComponent<C_Canvas>();

	a = App->scene->GetGameObjectByName(creditsTextName.c_str());
	if (a != nullptr)
		creditsText = (C_UI_Text*)a->GetComponent<C_UI_Text>();

	a = App->scene->GetGameObjectByName(beskarTextName.c_str());
	if (a != nullptr)
		beskarText = (C_UI_Text*)a->GetComponent<C_UI_Text>();

	a = App->scene->GetGameObjectByName(ammoTextName.c_str());
	if (a != nullptr)
		ammoText = (C_UI_Text*)a->GetComponent<C_UI_Text>();

	playerObject = App->scene->GetGameObjectByName(playerName.c_str());
	if(playerObject != nullptr)
		player = (Player*)playerObject->GetScript("Player");
}

void HUDManager::Update()
{
	//Pau Pedra did this
	if(debugMenuCanvas != nullptr && hubShopCanvas != nullptr && hudCanvas != nullptr && pauseMenuCanvas != nullptr)
		if (debugMenuCanvas->IsActive() || hubShopCanvas->IsActive() || pauseMenuCanvas->IsActive())
		{
			if (hudCanvas->IsActive())
				hudCanvas->SetIsActive(false);
		}
		else
		{
			if(!hudCanvas->IsActive())
				hudCanvas->SetIsActive(true);
		}

	if (beskarText != nullptr)
	{
		if (player != nullptr)
		{
			std::string tmp = "";
			tmp += std::to_string(player->hubCurrency).c_str();
			beskarText->SetText(tmp.c_str());
		}
	}

	if (creditsText != nullptr)
	{
		if (player != nullptr)
		{
			std::string tmp = "";
			tmp += std::to_string(player->currency).c_str();
			creditsText->SetText(tmp.c_str());
		}
	}

	if (ammoText != nullptr)
	{
		if (player != nullptr)
		{
			std::string tmp = std::to_string(player->GetCurrentWeapon()->ammo);
			tmp += " / " ;
			tmp += std::to_string(player->GetCurrentWeapon()->maxAmmo);
			ammoText->SetText(tmp.c_str());
		}
	}

	//Santi Did this
	if(player != nullptr)
	{
		
		//Take damage animation
		if (mandoImage != nullptr)
		{
			if (player->hitTimer.IsActive())
				mandoImage->PlayAnimation(false, 1);
		}

		//Reload primary weapon
		if (primaryWeaponImage != nullptr)
		{
			if (App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				primaryWeaponImage->PlayAnimation(false, 2);
		}

		//Shoot primary weapon
		if (primaryWeaponImage != nullptr)
		{
			if (App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
				primaryWeaponImage->PlayAnimation(false, 1);
		}

		//Swap primary weapon
		if (primaryWeaponImage != nullptr)
		{
			if (App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN)
				primaryWeaponImage->PlayAnimation(false, 3);
		}
/*
		//Reload secondary weapon
		if (secondaryWeaponImage != nullptr)
		{
			if (player)
				secondaryWeaponImage->PlayAnimation(false, 1);
		}

		//Shoot secondary weapon
		if (secondaryWeaponImage != nullptr)
		{
			if (player)
				secondaryWeaponImage->PlayAnimation(false, 1);
		}

		//Swap secondary weapon
		if (secondaryWeaponImage != nullptr)
		{
			if (player)
				secondaryWeaponImage->PlayAnimation(false, 1);
		}
*/
		//Get credit animation
		if (creditsImage != nullptr)
		{
			if (player)
				creditsImage->PlayAnimation(false, 1);
		}
			//Dash animation
		if (dashImage != nullptr)
		{
			if (App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN)
				dashImage->PlayAnimation(false, 1);
		}
	}
}
