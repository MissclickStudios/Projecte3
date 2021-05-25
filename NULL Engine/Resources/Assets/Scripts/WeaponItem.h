#pragma once
#include "Object.h"
#include "ScriptMacros.h"

#include "Prefab.h"

#include <vector>
#include <string>

class ItemMenuManager;

class Player;

class SCRIPTS_API WeaponItem : public Object ALLOWED_INHERITANCE
{
public:

	WeaponItem();
	virtual ~WeaponItem();

	void Awake() override;
	void Update() override;
	void CleanUp() override;

	void OnPause() override;
	void OnResume() override;

	void OnTriggerRepeat(GameObject* object) override;

	void PickUp(Player* player);

	std::string gunName;
	std::string gunDescription;

	Prefab weapon;

	std::string itemMenuName = "Item Menu";

protected:

	ItemMenuManager* itemMenu = nullptr;
};

SCRIPTS_FUNCTION WeaponItem* CreateWeaponItem();