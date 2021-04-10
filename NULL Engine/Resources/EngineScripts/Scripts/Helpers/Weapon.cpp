#include "Application.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_Script.h"
#include "C_AudioSource.h"

#include "Bullet.h"

#include "Weapon.h"

Weapon::Weapon(GameObject* shooter, Prefab projectilePrefab, int projectilesNum, int maxAmmo, float projectileSpeed, float fireRate, bool automatic)
{
	this->shooter = shooter;
	this->projectilePrefab = projectilePrefab;

	this->projectilesNum = projectilesNum;
	this->maxAmmo = maxAmmo;
	ammo = maxAmmo;

	this->projectileSpeed = projectileSpeed;
	this->fireRate = fireRate;
	fireRateTimer.Stop();

	this->automatic = automatic;
}

Weapon::~Weapon()
{
	if (projectileStorage || projectiles)
		CleanUp();
}

void Weapon::Update()
{
	if (!projectileStorage)
	{
		if (projectiles)
			DeleteProjectiles();
		projectiles = new Projectile * [projectilesNum];

		projectileStorage = App->scene->CreateGameObject("Bullets", App->scene->GetSceneRoot());
		for (uint i = 0; i < projectilesNum; ++i)
			projectiles[i] = CreateProjectile(i);
	}
}

void Weapon::CleanUp()
{
	if (projectiles)
		DeleteProjectiles();
	if (projectileStorage)
		DeleteProjectileStorage();
}

bool Weapon::Shoot(float3 direction)
{
	if (ammo > 0)
	{
		if (!automatic)
		{
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_DOWN || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_DOWN)
			{
				FireProjectile(direction);
			}

			return true;
		}
		else
		{
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
			{
				if (!fireRateTimer.IsActive())
				{
					FireProjectile(direction);
					fireRateTimer.Start();
				}
				else if (fireRateTimer.ReadSec() >= fireRate)
				{
					FireProjectile(direction);
					fireRateTimer.Stop();
					fireRateTimer.Start();
				}

				return true;
			}
		}
	}
	return false;
}

Projectile* Weapon::CreateProjectile(uint index)
{
	GameObject* projectile = App->resourceManager->LoadPrefab(projectilePrefab.uid, projectileStorage);
	projectile->GetComponent<C_BoxCollider>()->Update();

	char n[10];
	sprintf_s(n, "%d", index);
	std::string num = n;
	std::string name("Projectile" + num);

	projectile->SetName(name.c_str());

	float3 position = float3::zero;
	projectile->transform->SetWorldPosition(position);

	((Bullet*)projectile->GetComponent<C_Script>()->GetScriptData())->SetShooter(shooter, index);

	for (uint i = 0; i < projectile->components.size(); ++i)
		projectile->components[i]->SetIsActive(false);
	projectile->SetIsActive(false);

	return new Projectile(projectile);
}

void Weapon::FireProjectile(float3 direction)
{
	if (!projectileStorage || !projectiles)
		return;

	if (direction.IsZero())
		++direction.z;

	GameObject* projectile = nullptr;

	for (uint i = 0; i < projectilesNum; ++i)
		if (!projectiles[i]->inUse)
		{
			projectiles[i]->inUse = true;
			projectile = projectiles[i]->object;
			break;
		}
	if (!projectile)
		return;

	float3 position = shooter->transform->GetWorldPosition();
	position.y += 4;
	projectile->transform->SetWorldPosition(position);

	float2 dir = { direction.x, -direction.z };
	float rad = dir.AimedAngle();
	projectile->transform->SetLocalRotation(float3(0, rad + DegToRad(90), 0));

	C_RigidBody* rigidBody = projectile->GetComponent<C_RigidBody>();
	if (rigidBody)
	{
		rigidBody->TransformMovesRigidBody(true);
		rigidBody->SetLinearVelocity(direction * projectileSpeed);
	}

	C_AudioSource* source = projectile->GetComponent<C_AudioSource>();
	if (source)
		source->PlayFx(source->GetEventId());

	projectile->SetIsActive(true);
	for (uint i = 0; i < projectile->components.size(); ++i)
		projectile->components[i]->SetIsActive(true);

	--ammo;
}

void Weapon::Reload()
{
	ammo = maxAmmo;
}

void Weapon::DisableProjectile(uint index)
{
	projectiles[index]->inUse = false;
}

bool Weapon::SandTrooperShoot(float3 direction, float modifier)
{
	if (ammo > 0)
	{
		if (!automatic)
		{
			FireProjectile(direction);

			return true;
		}
		else
		{
			if (!fireRateTimer.IsActive())
			{
				FireProjectile(direction);
				fireRateTimer.Start();

				return true;
			}
			else if (fireRateTimer.ReadSec() >= fireRate / modifier)
			{
				FireProjectile(direction);
				fireRateTimer.Stop();
				fireRateTimer.Start();

				return true;
			}
		}
	}
	return false;
}

void Weapon::DeleteProjectiles()
{
	for (uint i = 0; i < projectilesNum; ++i)
	{
		if (projectiles[i])
			delete projectiles[i];
		projectiles[i] = nullptr;
	}
	delete projectiles;
	projectiles = nullptr;
}

void Weapon::DeleteProjectileStorage()
{
	projectileStorage->to_delete = true;
	projectileStorage = nullptr;
}