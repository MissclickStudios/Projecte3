#include "MC_Time.h"

#include "WeaponItem.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Canvas.h"

#include "ItemMenuManager.h"

#include "Player.h"

WeaponItem::WeaponItem() : Object()
{
	baseType = ObjectType::WEAPON_ITEM;
}

WeaponItem::~WeaponItem()
{
}

void WeaponItem::Awake()
{
	GameObject* gameObject = App->scene->GetGameObjectByName(itemMenuName.c_str());
	if (gameObject != nullptr)
		itemMenu = (ItemMenuManager*)gameObject->GetScript("ItemMenuManager");
}

void WeaponItem::Update()
{
	//gameObject->transform->Translate(float3::one * sin(MC_Time::Game::GetTimeSinceStart()));
	gameObject->transform->Rotate(rotationSpeed * MC_Time::Game::GetDT());
}

void WeaponItem::CleanUp()
{
}

void WeaponItem::OnPause()
{
}

void WeaponItem::OnResume()
{
}

void WeaponItem::OnTriggerRepeat(GameObject* object)
{
	if (weapon.uid == NULL)
		return;

	itemMenu->SetWeapon(weapon, gameObject->transform->GetWorldPosition(), gunName, gunDescription);
}

void WeaponItem::PickUp(Player* player)
{
	player->EquipWeapon(weapon);
	Deactivate();
}

SCRIPTS_FUNCTION WeaponItem* CreateWeaponItem()
{
	WeaponItem* script = new WeaponItem();
	
	INSPECTOR_INPUT_FLOAT(script->hoverSpeed);
	INSPECTOR_INPUT_FLOAT(script->hoverRange);
	
	INSPECTOR_INPUT_FLOAT3(script->rotationSpeed);

	INSPECTOR_STRING(script->gunName);
	INSPECTOR_STRING(script->gunDescription);

	INSPECTOR_PREFAB(script->weapon);

	return script;
}
