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
	Weapon* const weapon = player->GetCurrentWeapon();
	if (weapon)
		weapon->AddPerk(perk);
}

void WeaponPerk::SetTypeInfo()
{
}

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk()
{
	WeaponPerk* script = new WeaponPerk();

	INSPECTOR_DRAGABLE_INT(script->price);

	INSPECTOR_TEXT(script->typeName);
	INSPECTOR_SLIDER_INT(script->typeInt, 0, (int)Perk::PERKS_NUM - 1);

	return script;
}
