#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float2.h"

class Player;

class SCRIPTS_API WeaponPerk : public Script
{
public:

	WeaponPerk();
	~WeaponPerk();

	void Update() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	int cost = 200;
	int effect = 1;

private:

	bool used = false;
};

SCRIPTS_FUNCTION WeaponPerk* CreateWeaponPerk() {
	WeaponPerk* script = new WeaponPerk();

	INSPECTOR_DRAGABLE_INT(script->cost);
	INSPECTOR_SLIDER_INT(script->effect, 1, 2);

	return script;
}