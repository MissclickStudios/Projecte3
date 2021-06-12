#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Text.h"
#include "C_UI_Image.h"
#include "C_2DAnimator.h"
#include "GameObject.h"
#include "HUDManager.h"
#include "Player.h"
#include "M_Input.h"
#include "M_ResourceManager.h"
#include "M_UISystem.h"
#include "Weapon.h"
#include "M_Scene.h"



HUDManager::HUDManager() : Script()
{
}

HUDManager::~HUDManager()
{
}

void HUDManager::Start()
{
	for (std::vector<std::string>::const_iterator it = disabledScenes.cbegin(); it != disabledScenes.cend();++it) 
	{
		if (!strcmp(App->scene->GetCurrentScene(), (*it).c_str()))
		{
			enabled = false;
			break;
		}
	}
	if (enabled)
	{
		GameObject* obj = nullptr;
		for (int i = 0; i < MAX_HEARTS; ++i)
		{
			obj = App->scene->GetGameObjectByName(std::string("Heart" + std::to_string(i + 1)).c_str());
			if (!obj)
			{
				enabled = false;
				break;
			}
			hearts[i] = obj->GetComponent<C_UI_Image>();
			if (!hearts[i])
			{
				enabled = false;
				break;
			}
			else
				heartsAnimations[i] = obj->GetComponent<C_2DAnimator>();
		}
	}
	if (enabled) 
	{
		GameObject* obj = App->scene->GetGameObjectByName(mandoImageName.c_str());
		if (obj != nullptr)
			mandoImage = (C_2DAnimator*)obj->GetComponent<C_2DAnimator>();

		obj = App->scene->GetGameObjectByName(primaryWeaponImageName.c_str());
		if (obj != nullptr)
			primaryWeaponImage = (C_2DAnimator*)obj->GetComponent<C_2DAnimator>();

		obj = App->scene->GetGameObjectByName(secondaryWeaponImageName.c_str());
		if (obj != nullptr)
			secondaryWeaponImage = (C_2DAnimator*)obj->GetComponent<C_2DAnimator>();

		obj = App->scene->GetGameObjectByName(dashImageName.c_str());
		if (obj != nullptr)
			dashImage = (C_2DAnimator*)obj->GetComponent<C_2DAnimator>();

		obj = App->scene->GetGameObjectByName(creditsImageName.c_str());
		if (obj != nullptr)
			creditsImage = (C_2DAnimator*)obj->GetComponent<C_2DAnimator>();

		hudCanvas = (C_Canvas*)gameObject->GetComponent<C_Canvas>();
		if (hudCanvas)
			App->uiSystem->PushCanvas(hudCanvas);

		obj = App->scene->GetGameObjectByName(creditsTextName.c_str());
		if (obj != nullptr)
			creditsText = (C_UI_Text*)obj->GetComponent<C_UI_Text>();

		obj = App->scene->GetGameObjectByName(beskarTextName.c_str());
		if (obj != nullptr)
			beskarText = (C_UI_Text*)obj->GetComponent<C_UI_Text>();

		obj = App->scene->GetGameObjectByName(ammoTextName.c_str());
		if (obj != nullptr)
			ammoText = (C_UI_Text*)obj->GetComponent<C_UI_Text>();

		playerObject = App->scene->GetGameObjectByName(playerName.c_str());
		if (playerObject != nullptr)
			player = (Player*)playerObject->GetScript("Player");
			
		obj = App->scene->GetGameObjectByName(weapon1Name.c_str());
		if (obj != nullptr)
			weapon1 = obj;

		obj = App->scene->GetGameObjectByName(weapon2Name.c_str());
		if (obj != nullptr)
			weapon2 = obj;

		obj = App->scene->GetGameObjectByName(weapon3Name.c_str());
		if (obj != nullptr)
			weapon3 = obj;

		obj = App->scene->GetGameObjectByName(weapon4Name.c_str());
		if (obj != nullptr)
			weapon4 = obj;

		hitAlready = false;
	}
}

void HUDManager::Update()
{
	if (enabled) 
	{
		if (setUpHealth)
		{
			SetUpHealth(player->health ,player->MaxHealth());
			setUpHealth = false;
		}
		if (beskarText != nullptr && player != nullptr)
		{
			std::string tmp = "";
			tmp += std::to_string(player->beskar).c_str();
			beskarText->SetText(tmp.c_str());
		}

		if (creditsText != nullptr && player != nullptr)
		{

			std::string tmp = "";
			tmp += std::to_string(player->credits).c_str();
			creditsText->SetText(tmp.c_str());
		}

		if (ammoText != nullptr && player != nullptr)
		{
			Weapon* weapon = player->GetCurrentWeapon();
			if (weapon)
			{
				std::string tmp = std::to_string(weapon->ammo);
				tmp += " / ";
				tmp += std::to_string(weapon->MaxAmmo());
				ammoText->SetText(tmp.c_str());
			}
		}

		//Santi Did this
		if (player != nullptr)
		{
			//Take damage animation
			if (player->hitTimer.IsActive() && !hitAlready)
			{
				if (mandoImage)
					mandoImage->PlayAnimation(false, 1);
				if (heartsAnimations[currentHealthIndex])
				{
					if ((int)player->health % 2 == 0)
						heartsAnimations[currentHealthIndex]->PlayAnimation(false, 1);
					else
						heartsAnimations[currentHealthIndex]->PlayAnimation(false, 2);
				}
				if ((int)player->health % 2 == 0)
				{
					hearts[currentHealthIndex]->SetTextureCoordinates(emptyHeart[0], emptyHeart[1], emptyHeart[2], emptyHeart[3]);
					--currentHealthIndex;
				}
				else 
					hearts[currentHealthIndex]->SetTextureCoordinates(halfHeart[0], halfHeart[1], halfHeart[2], halfHeart[3]);

				playerHealth -= 1;
				hitAlready = true;
			}
			if (!player->hitTimer.IsActive())
				hitAlready = false;

			if (playerMaxHp != player->MaxHealth() || playerHealth != player->health)
				SetUpHealth(player->health, player->MaxHealth());

			/*
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
			*/
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
		}
	}
}

void HUDManager::CleanUp()
{
}

void HUDManager::ManageWeaponHUD()
{
	switch (type) 
	{
	case WeaponType::BLASTER:

		break;
	case WeaponType::MINIGUN:

		break;
	case WeaponType::SHOTGUN:

		break;
	case WeaponType::SNIPER:

		break;
	case WeaponType::WEAPON:

		break;
	}

}

void HUDManager::SetUpHealth(float hp, int maxHp)
{
	if (hp < 0)
		hp = 0;
	playerHealth = hp;
	playerMaxHp = maxHp;
	bool eaven = !((int)hp % 2);
	if (eaven && hp !=0) 
	{
		currentHealthIndex = hp / 2 - 1;
		if (hearts[currentHealthIndex] != nullptr)
			hearts[currentHealthIndex]->SetTextureCoordinates(fullHeart[0], fullHeart[1], fullHeart[2], fullHeart[3]);
	}
	else 
	{
		currentHealthIndex = hp / 2;
		if (hearts[currentHealthIndex] != nullptr)
			hearts[currentHealthIndex]->SetTextureCoordinates(halfHeart[0], halfHeart[1], halfHeart[2], halfHeart[3]);
	}

	for (int i = 0; i < currentHealthIndex; ++i)
		if (hearts[i] != nullptr)
			hearts[i]->SetTextureCoordinates(fullHeart[0], fullHeart[1], fullHeart[2], fullHeart[3]);
	for (int i = currentHealthIndex+1; i < (playerMaxHp / 2); ++i)
		if (hearts[i] != nullptr)
			hearts[i]->SetTextureCoordinates(emptyHeart[0], emptyHeart[1], emptyHeart[2], emptyHeart[3]);
	for (int i = (playerMaxHp/2); i < MAX_HEARTS; ++i)
		if (hearts[i] != nullptr)
			hearts[i]->SetTextureCoordinates(noRender[0], noRender[1], noRender[2], noRender[3]);
}

/*C_2DAnimator* HUDManager::GetHeartAnimation(int index)
{
	switch (index)
	{
	case 0:
		return heart1Anim;
	case 1:
		return heart2Anim;
	case 2:
		return heart3Anim;
	case 3:
		return heart4Anim;
	case 4:
		return heart5Anim;
	case 5:
		return heart6Anim;
	default:
		return nullptr;
	}
}*/

HUDManager* CreateHUDManager()
{
	HUDManager* script = new HUDManager();
	INSPECTOR_STRING(script->mandoImageName);
	INSPECTOR_STRING(script->secondaryWeaponImageName);
	INSPECTOR_STRING(script->primaryWeaponImageName);
	INSPECTOR_STRING(script->dashImageName);
	INSPECTOR_STRING(script->creditsImageName);
	INSPECTOR_STRING(script->playerName);

	INSPECTOR_STRING(script->creditsTextName);
	INSPECTOR_STRING(script->beskarTextName);
	INSPECTOR_STRING(script->ammoTextName);

	INSPECTOR_STRING(script->weapon1Name);
	INSPECTOR_STRING(script->weapon2Name);
	INSPECTOR_STRING(script->weapon3Name);
	INSPECTOR_STRING(script->weapon4Name);

	INSPECTOR_VECTOR_STRING(script->disabledScenes);
	return script;
}