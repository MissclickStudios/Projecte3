#pragma once
#include "Script.h"

#include "Prefab.h"
#include "Timer.h"

#include "Effect.h"

#include "MathGeoLib/include/Math/float2.h"

#include <vector>

#define DEFAULT_MODIFIER 1.0f

struct Projectile;

enum class WeaponType
{
	WEAPON,
	BLASTER,
	SNIPER,
	SHOTGUN
};

enum class ShootState
{
	NO_FULLAUTO, // seems like rules specify that fullauto is not permited indoors, but wait...
	WAINTING_FOR_NEXT,
	FIRED_PROJECTILE,
	NO_AMMO
};

class  Weapon : public Script
{
public:

	Weapon();
	virtual ~Weapon();

	void Start();
	virtual void SetUp() = 0;
	void Update();
	void CleanUp();

	void Activate();
	void Deactivate();

	// Usability
	virtual ShootState Shoot(float2 direction);
	virtual ShootState ShootLogic() = 0;
	virtual bool Reload();
	void ProjectileCollisionReport(int index);
	virtual std::vector<Effect> GetOnHitEffects();

	// Type
	WeaponType type = WeaponType::WEAPON;

	// Stats
	// Shoot
	float damage = 0.0f;
	float Damage() { return damage * damageModifier; }
	float projectileSpeed = 0.0f;
	float ProjectileSpeed() { return projectileSpeed * projectileSpeedModifier; }
	float fireRate = 0.0f;
	float FireRate() { return fireRate * fireRateModifier; }
	int ammo = 0;
	int maxAmmo = 0;
	int MaxAmmo() { return maxAmmo + maxAmmoModifier; }
	int projectilesPerShot = 0;

	GameObject* hand = nullptr;
	// Reload
	float reloadTime = 0.0f;

	// Modifiers
	float damageModifier = DEFAULT_MODIFIER;
	float projectileSpeedModifier = DEFAULT_MODIFIER;
	float fireRateModifier = DEFAULT_MODIFIER;
	int maxAmmoModifier = DEFAULT_MODIFIER;

	// Prefabs
	Prefab weaponModel;
	Prefab projectilePrefab;

	// Projectiles
	int projectileNum = 0;
	bool updateProjectiles = false;

	// Inspector
	void InspectorCalls();

protected:

	void CreateProjectiles();
	void DeleteProjectiles();
	virtual void FireProjectile(float2 direction);

	// Shoot
	Timer fireRateTimer;
	// Reload
	Timer reloadTimer;

	GameObject* projectileHolder = nullptr;
	Projectile** projectiles = nullptr;
};