#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
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
	mandoImage = (C_2DAnimator*)App->scene->GetGameObjectByName(mandoImageName.c_str())->GetComponent<C_2DAnimator>();
	primaryWeaponImage = (C_2DAnimator*)App->scene->GetGameObjectByName(primaryWeaponImageName.c_str())->GetComponent<C_2DAnimator>();
	secondaryWeaponImage = (C_2DAnimator*)App->scene->GetGameObjectByName(secondaryWeaponImageName.c_str())->GetComponent<C_2DAnimator>();
	dashImage = (C_2DAnimator*)App->scene->GetGameObjectByName(dashImageName.c_str())->GetComponent<C_2DAnimator>();
	creditsImage = (C_2DAnimator*)App->scene->GetGameObjectByName(creditsImageName.c_str())->GetComponent<C_2DAnimator>();

	playerObject = App->scene->GetGameObjectByName(playerName.c_str());
	player = (Player*)playerObject->GetScript("Player");
}

void HUDManager::Update()
{
	if(player != nullptr)
	{
		
		//Take damage animation
		if (mandoImage != nullptr)
		{
			if (player)
				mandoImage->PlayAnimation(false, 1);
		}

		//Reload primary weapon
		if (primaryWeaponImage != nullptr)
		{
			if (App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				primaryWeaponImage->PlayAnimation(false, 1);
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
				primaryWeaponImage->PlayAnimation(false, 1);
		}

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
