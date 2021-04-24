/*#include "Application.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_2DAnimator.h"
#include "GameObject.h"
#include "HUDManager.h"
#include "Player.h"

HUDManager::HUDManager()
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

	player = App->scene->GetGameObjectByName(playerName.c_str());
}

void HUDManager::Update()
{
	if(player != nullptr)
	{
	}
}*/
