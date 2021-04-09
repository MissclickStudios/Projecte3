#pragma once
#include "Prefab.h"
#include "Time.h"

#include "Projectile.h"

#include "MathGeoLib/include/Math/float3.h"

class Weapon
{
public:

	Weapon(GameObject* shooter, Prefab projectilePrefab, int projectilesNum, int maxAmmo, float projectileSpeed, float fireRate, bool automatic);
	~Weapon();

	void Update();
	void CleanUp();

	bool Shoot(float3 direction);

	void DisableProjectile(uint index);

	float projectileSpeed = 100.0f;
	float fireRate = 0.25f;

	int ammo = 10;
	int maxAmmo = 10;

	bool automatic = true;

	Prefab projectilePrefab;

	GameObject* projectileStorage = nullptr;
	uint projectilesNum = 10;
	Projectile** projectiles;

private:

	Projectile* CreateProjectile(uint index);
	void FireProjectile(float3 direction);
	void Reload();

	void DeleteProjectiles();
	void DeleteProjectileStorage();

	Timer fireRateTimer;
	GameObject* shooter;
};