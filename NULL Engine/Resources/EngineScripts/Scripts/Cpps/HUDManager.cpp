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
#include "M_ResourceManager.h"
#include "C_Material.h"
#include "R_Texture.h"
#include "M_UISystem.h"


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

	hudCanvas = (C_Canvas*)gameObject->GetComponent<C_Canvas>();
	if(hudCanvas)
		App->uiSystem->PushCanvas(hudCanvas);

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

	a = App->scene->GetGameObjectByName(heart1Name.c_str());
	if (a != nullptr)
		heart1 = (C_Material*)a->GetComponent<C_Material>();

	a = App->scene->GetGameObjectByName(heart2Name.c_str());
	if (a != nullptr)
		heart2 = (C_Material*)a->GetComponent<C_Material>();

	a = App->scene->GetGameObjectByName(heart3Name.c_str());
	if (a != nullptr)
		heart3 = (C_Material*)a->GetComponent<C_Material>();

	a = App->scene->GetGameObjectByName(heart1Name.c_str());
	if (a != nullptr)
		heart1Image = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(heart2Name.c_str());
	if (a != nullptr)
		heart2Image = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();

	a = App->scene->GetGameObjectByName(heart3Name.c_str());
	if (a != nullptr)
		heart3Image = (C_2DAnimator*)a->GetComponent<C_2DAnimator>();


	fullHeart = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartFull.png");
	emptyHeart = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartEmpty.png");
	halfHeart = (R_Texture*)App->resourceManager->GetResourceFromLibrary("Assets/Textures/UI/HUD/HeartHalf.png");

	health1 = false;
	health2 = false;
	health3 = false;
	health4 = false;
	health5 = false;
	health6 = false;

	hitAlready = false;
}

void HUDManager::Update()
{

	if (beskarText != nullptr && player != nullptr)
	{
		std::string tmp = "";
		tmp += std::to_string(player->hubCurrency).c_str();
		beskarText->SetText(tmp.c_str());
	}

	if (creditsText != nullptr && player != nullptr)
	{

		std::string tmp = "";
		tmp += std::to_string(player->currency).c_str();
		creditsText->SetText(tmp.c_str());
	}

	if (ammoText != nullptr && player != nullptr)
	{
		Weapon* weapon = player->GetCurrentWeapon();
		if (weapon)
		{
			std::string tmp = std::to_string(weapon->ammo);
			tmp += " / ";
			tmp += std::to_string(weapon->maxAmmo);
			ammoText->SetText(tmp.c_str());
		}
	}

	//Santi Did this
	if(player != nullptr)
	{
		
		//Take damage animation
		if (mandoImage != nullptr)
		{
			if (player->hitTimer.IsActive() && !hitAlready)
			{
				mandoImage->PlayAnimation(false, 1);
				hitAlready = true;
			}
		}

		//Reload primary weapon
		if (primaryWeaponImage != nullptr)
		{
			if (App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN)
				primaryWeaponImage->PlayAnimation(false, 3);
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
				primaryWeaponImage->PlayAnimation(false, 2);
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
	//	if (dashImage != nullptr)
	//	{
	//		if (App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN)
	//			dashImage->PlayAnimation(false, 1);
	//	}

		if (!player->hitTimer.IsActive())
			hitAlready = false;

		ManageHeartImage(player->health);
	}
}

void HUDManager::CleanUp()
{
	if (halfHeart != nullptr)
		App->resourceManager->FreeResource(halfHeart->GetUID());
	if (fullHeart != nullptr)
		App->resourceManager->FreeResource(fullHeart->GetUID());
	if (emptyHeart != nullptr)
		App->resourceManager->FreeResource(emptyHeart->GetUID());
}

void HUDManager::ManageHeartImage(int hp)
{

	switch(hp) 
	{
	case 0:
		heart1->SwapTexture(emptyHeart);
		heart2->SwapTexture(emptyHeart);
		heart3->SwapTexture(emptyHeart);
		if (!health1)
		{
			heart1Image->PlayAnimation(false, 1);
			health1 = true;
		}
		health2 = false;
		health3 = false;
		health4 = false;
		health5 = false;
		health6 = false;
		break;
	case 1:	  
		heart1->SwapTexture(halfHeart);
		heart2->SwapTexture(emptyHeart);
		heart3->SwapTexture(emptyHeart);
		if (!health2)
		{
		heart1Image->PlayAnimation(false, 2);
		health2 = true;
		}
		health1 = false;
		health3 = false;
		health4 = false;
		health5 = false;
		health6 = false;
		break;
	case 2:	  
		heart1->SwapTexture(fullHeart);
		heart2->SwapTexture(emptyHeart);
		heart3->SwapTexture(emptyHeart);
		if (!health3)
		{
		heart2Image->PlayAnimation(false, 1);
		health3 = true;
		}
		health1 = false;
		health2 = false;
		health4 = false;
		health5 = false;
		health6 = false;
		break;
	case 3:	 
		heart1->SwapTexture(fullHeart);
		heart2->SwapTexture(halfHeart);
		heart3->SwapTexture(emptyHeart);
		if (!health4)
		{
		heart2Image->PlayAnimation(false, 2);
		health4 = true;
		}
		health1 = false;
		health2 = false;
		health3 = false;
		health5 = false;
		health6 = false;
		break;
	case 4:	  
		heart1->SwapTexture(fullHeart);
		heart2->SwapTexture(fullHeart);
		heart3->SwapTexture(emptyHeart);
		if (!health5)
		{
		heart3Image->PlayAnimation(false, 1);
		health5 = true;
		}
		health1 = false;
		health2 = false;
		health3 = false;
		health4 = false;
		health6 = false;
		break;
	case 5:	  
		heart1->SwapTexture(fullHeart);
		heart2->SwapTexture(fullHeart);
		heart3->SwapTexture(halfHeart);
		if (!health6)
		{
		heart3Image->PlayAnimation(false, 2);
		health6 = true;
		}
		health1 = false;
		health2 = false;
		health3 = false;
		health4 = false;
		health5 = false;
		break;
	case 6:		
		heart1->SwapTexture(fullHeart);
		heart2->SwapTexture(fullHeart);
		heart3->SwapTexture(fullHeart);
		break;
	}
}
