#include "WeaponPerk.h"

#include "Player.h"

WeaponPerk::WeaponPerk() : Collectable()
{
}

WeaponPerk::~WeaponPerk()
{
}

void WeaponPerk::Contact(Player* player)
{
	//Weapon* const weapon = player->GetCurrentWeapon();
	//if (weapon)
	//	weapon->AddPerk(perk);
}

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk()
{
	WeaponPerk* script = new WeaponPerk();

	INSPECTOR_DRAGABLE_INT(script->price);

	INSPECTOR_ENUM(script->perk, "Perk", "EngineScripts/Scripts/Helpers/Perk.h");

	return script;
}
