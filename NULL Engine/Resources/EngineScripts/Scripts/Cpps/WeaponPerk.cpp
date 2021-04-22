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

void WeaponPerk::SetTypeInt()
{
	typeInt = (int)perk;
}

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk()
{
	WeaponPerk* script = new WeaponPerk();

	INSPECTOR_SLIDER_INT(script->typeInt, 0, (int)Perk::PERKS_NUM);

	return script;
}
