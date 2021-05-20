#pragma once
#include "ScriptMacros.h"

#include "Collectable.h"
#include "Weapon.h"

class SCRIPTS_API WeaponPerk : public Collectable ALLOWED_INHERITANCE
{
public:

	WeaponPerk();
	virtual ~WeaponPerk();

	PerkType perk = PerkType::NONE;

protected:

	void Contact(Player* player);

	void SetTypeInfo();
};

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk();