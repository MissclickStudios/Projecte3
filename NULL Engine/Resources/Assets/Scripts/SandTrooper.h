#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"
#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class Weapon;

class SCRIPTS_API SandTrooper : public Script
{
public:

	SandTrooper();
	~SandTrooper();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void TakeDamage(float damage);

	// Movement
	float speed = 10.0f;

	float detectionRange = 40.0f;

	GameObject* player = nullptr;

	// Weapon
	float projectileSpeed = 100.0f;
	float fireRate = 0.5f;
	int ammo = 10;
	int maxAmmo = 10;
	bool automatic = true;
	float reloadTime = 3.0f;

	Weapon* weapon;
	Prefab projectilePrefab;

	// Health
	float health = 4.0f;
	float maxHealth = 4.0f;

	// Attack
	float damage = 0.5f;

private:

	float3 LookingAt();

	// Movement
	float distance = 10000.0f;	// Distance from the player

	float3 direction = float3::zero;

	Timer reloadTimer;
};


SCRIPTS_FUNCTION SandTrooper* CreateSandTrooper();