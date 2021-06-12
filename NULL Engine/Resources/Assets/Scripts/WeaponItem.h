#pragma once
#include <vector>
#include <string>

#include "MathGeoLib/include/Math/float3.h";

#include "Object.h"
#include "ScriptMacros.h"

#include "Prefab.h"

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

public:
	void PickUp(Player* player);

	void CalculateNewHoverPosition();

public:
	Prefab		weapon;
	
	std::string gunName				= "";
	std::string gunDescription		= "";
	std::string itemMenuName		= "Item Menu";

	float3		rotationSpeed		= float3::zero;

	float3		hoverSpeed			= float3::one;
	float3		hoverRange			= float3::one;

	float		timeOffset			= 0.0f;

protected:
	ItemMenuManager* itemMenu		= nullptr;

private:
	float3		newPosition			= float3::zero;
	float3		hoverRate			= float3::zero;

	float		offsetCount			= 0.0f;

	bool		addX				= true;
	bool		addY				= true;
	bool		addZ				= true;
};

SCRIPTS_FUNCTION WeaponItem* CreateWeaponItem();