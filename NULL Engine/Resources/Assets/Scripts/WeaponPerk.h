#pragma once
#include "ScriptMacros.h"

#include "Collectable.h"
#include "Weapon.h"

class SCRIPTS_API WeaponPerk : public Collectable ALLOWED_INHERITANCE
{
public:

	WeaponPerk();
	virtual ~WeaponPerk();

	Perk perk = Perk::DAMAGE_UP;

protected:

	void Contact(Player* player);

	void SetTypeInfo();
};

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk();