#pragma once
#include "Object.h"

#include "Prefab.h"
#include "Timer.h"

#include "Perk.h"
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

class  Weapon : public Object
{
public:

	Weapon();
	virtual ~Weapon();

	void Start() override;
	virtual void SetUp() = 0;
	void Update() override;
	void CleanUp() override;

	void Activate();
	void Deactivate();

	// Usability
	virtual ShootState Shoot(float2 direction);
	virtual bool Reload();

	// Bullet
	void ProjectileCollisionReport(int index);

	// Perks
	void RefreshPerks(); // Resets the modifiers and applies all the current perks
	void AddPerk(Perk perk);

	virtual void DamageUp();
	virtual void MaxAmmoUp();
	virtual void FireRateUp();
	virtual void FastReload();
	virtual void FreezeBullets();

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
	float bulletLifeTime = 0.0f;
	float BulletLifeTime() { return bulletLifeTime / bulletLifeTimeModifier; }
	int ammo = 0;
	int maxAmmo = 0;
	int MaxAmmo() { return maxAmmo + maxAmmoModifier; }
	int projectilesPerShot = 0;

	GameObject* hand = nullptr;
	// Reload
	float reloadTime = 0.0f;
	float ReloadTime() { return reloadTime / reloadTimeModifier; }

	// Modifiers
	float damageModifier = DEFAULT_MODIFIER;
	float projectileSpeedModifier = DEFAULT_MODIFIER;
	float fireRateModifier = DEFAULT_MODIFIER;
	float reloadTimeModifier = DEFAULT_MODIFIER;
	float bulletLifeTimeModifier = DEFAULT_MODIFIER;
	int maxAmmoModifier = 0.0f;
	int PPSModifier = 0.0f;

	// Perks - most condecorated league player in the west btw, no cringe intended
	std::vector<Perk> perks;
	std::vector<Effect> onHitEffects;

	// Prefabs
	Prefab weaponModel;
	Prefab projectilePrefab;

	// Projectiles
	int projectileNum = 0;
	bool updateProjectiles = false;

protected:

	virtual ShootState ShootLogic() = 0;

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