#include "MC_Time.h"
#include "Log.h"
#include "EasingFunctions.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Canvas.h"

#include "ItemMenuManager.h"

#include "Player.h"

#include "WeaponItem.h"

WeaponItem::WeaponItem() : Object()
{
	baseType = ObjectType::WEAPON_ITEM;
}

WeaponItem::~WeaponItem()
{
}

void WeaponItem::Awake()
{
	if (gameObject != nullptr)
		gameObject->transform->SetLocalPosition(gameObject->transform->GetLocalPosition() - hoverRange);
	
	GameObject* menuManagerGO = App->scene->GetGameObjectByName(itemMenuName.c_str());
	if (menuManagerGO != nullptr)
		itemMenu = (ItemMenuManager*)menuManagerGO->GetScript("ItemMenuManager");
}

void WeaponItem::Update()
{
	if (offsetCount < timeOffset)
	{
		offsetCount += MC_Time::Game::GetDT();
		return;
	}

	CalculateNewHoverPosition();

	gameObject->transform->Translate(newPosition);
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

void WeaponItem::CalculateNewHoverPosition()
{
	newPosition = float3::zero;

	if (hoverRate.x >= 1.0f || hoverRate.x <= -1.0f) { addX = !addX; }					// --- All this mess just to avoid using sin() 3 times per frame.
	if (hoverRate.y >= 1.0f || hoverRate.y <= -1.0f) { addY = !addY; }					// 
	if (hoverRate.z >= 1.0f || hoverRate.z <= -1.0f) { addZ = !addZ; }					// 
	
	float dt = MC_Time::Game::GetDT();													// 
	hoverRate.x += (((addX) ? dt : -dt)) * hoverSpeed.x;								// It works tho :).
	hoverRate.y += (((addY) ? dt : -dt)) * hoverSpeed.y;								// And it hovers cool enough :).
	hoverRate.z += (((addZ) ? dt : -dt)) * hoverSpeed.z;								// 
	
	newPosition.x = (hoverRange.x * hoverRate.x) * 0.1f;								// 
	newPosition.y = (hoverRange.y * hoverRate.y) * 0.1f;								// 
	newPosition.z = (hoverRange.z * hoverRate.z) * 0.1f;								// --------------------------------------------------------------
}

SCRIPTS_FUNCTION WeaponItem* CreateWeaponItem()
{
	WeaponItem* script = new WeaponItem();
	
	INSPECTOR_INPUT_FLOAT3(script->rotationSpeed);
	INSPECTOR_INPUT_FLOAT3(script->hoverSpeed);
	INSPECTOR_INPUT_FLOAT3(script->hoverRange);
	
	INSPECTOR_INPUT_FLOAT(script->timeOffset);

	INSPECTOR_STRING(script->gunName);
	INSPECTOR_STRING(script->gunDescription);

	INSPECTOR_PREFAB(script->weapon);

	return script;
}
