#include "Weapon.h"

#include "Application.h"
#include "M_Physics.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"

#include "Bullet.h"

struct Projectile
{
	Projectile() : object(nullptr), inUse(false), bulletScript(nullptr) {}
	Projectile(GameObject* object) : object(object), inUse(false) { FindBulletScript(); }
	Projectile(GameObject* object, bool inUse) : object(object), inUse(inUse) { FindBulletScript(); }

	void FindBulletScript() { if (object) bulletScript = (Bullet*)object->GetScript("Bullet"); }

	GameObject* object;
	Bullet* bulletScript;
	bool inUse;
};

Weapon::Weapon() : Object()
{
	baseType = ObjectType::WEAPON;
}

Weapon::~Weapon()
{
}

void Weapon::Start()
{
	fireRateTimer.Stop();
	reloadTimer.Stop();

	for (uint i = 0; i < gameObject->components.size(); ++i)
	{
		if (gameObject->components[i]->GetType() == ComponentType::AUDIOSOURCE)
		{
			C_AudioSource* source = (C_AudioSource*)gameObject->components[i];
			std::string name = source->GetEventName();

			if (name == "weapon_laser_01")
				shootAudio = source;
			else if (name == "weapon_laser_02_02")
				shootAudio = source;
			else if (name == "weapon_reload_01")
				reloadAudio = source;
			else if (name == "weapon_reload_02")
				reloadAudio = source;
		}
	}
}

void Weapon::Update()
{
	if (updateProjectiles)
	{
		updateProjectiles = false;
		CreateProjectiles();
	}

	if (weaponModel)
	{
		weaponModel->transform->SetLocalPosition(position);
		weaponModel->transform->SetLocalEulerRotation(rotation);
		weaponModel->transform->SetLocalScale(scale);
	}
}

void Weapon::CleanUp()
{
	DeleteProjectiles();

	hand = nullptr;
}

ShootState Weapon::Shoot(float2 direction)
{
	ShootState state = ShootLogic();
	if (state == ShootState::FIRED_PROJECTILE)
	{
		FireProjectile(direction);

		ammo -= projectilesPerShot;
		if (ammo < 0)
			ammo = 0;

		if (shootAudio)
			shootAudio->PlayFx(shootAudio->GetEventId());
	}
	return state;
}

bool Weapon::Reload()
{
	if (!reloadTimer.IsActive())
		reloadTimer.Start();
	else if (reloadTimer.ReadSec() >= reloadTime)
	{
		reloadTimer.Stop();
		ammo = MaxAmmo();

		if (reloadAudio)
			reloadAudio->PlayFx(reloadAudio->GetEventId());

		return true;
	}
	return false;
}

void Weapon::SetOwnership(EntityType type, GameObject* hand)
{
	this->hand = hand;

	CreateProjectiles();
	RefreshPerks();

	SetUp();

	if (type == EntityType::PLAYER)
	{
		if (weaponModelPrefab.uid != NULL)
		{
			GameObject* skeletonHand = App->scene->GetGameObjectByName("mixamorig:RightHand");
			if (skeletonHand)
				weaponModel = App->resourceManager->LoadPrefab(weaponModelPrefab.uid, skeletonHand); // Load the prefab onto a gameobject
		}

		if (!projectiles)
			return;
		for (uint i = 0; i < projectileNum; ++i)
		{
			if (!projectiles[i])
				continue;
			if (!projectiles[i]->inUse)
			{
				C_RigidBody* rigidBody = projectiles[i]->object->GetComponent<C_RigidBody>();
				if (rigidBody)
					rigidBody->ChangeFilter(" bullet");
			}
		}
	}
}

void Weapon::ProjectileCollisionReport(int index)
{
	if (!projectiles)
		return;
	if (!projectiles[index])
		return;

	projectiles[index]->inUse = false;

	if (!projectiles[index]->object)
		return;
	projectiles[index]->object->transform->SetLocalPosition(float3::zero);
}

void Weapon::RefreshPerks()
{
	// Reset modifiers and on hit effects to avoid overwritting
	damageModifier = DEFAULT_MODIFIER;
	projectileSpeedModifier = DEFAULT_MODIFIER;
	fireRateModifier = DEFAULT_MODIFIER;
	reloadTimeModifier = DEFAULT_MODIFIER;
	maxAmmoModifier = 0.0f;
	PPSModifier = 0.0f;
	onHitEffects.clear();

	// Apply each perk
	for (uint i = 0; i < perks.size(); ++i)
		switch (perks[i])
		{
		case Perk::DAMAGE_UP:
			DamageUp();
			break;
		case Perk::MAXAMMO_UP:
			MaxAmmoUp();
			break;
		case Perk::FIRERATE_UP:
			FireRateUp();
			break;
		case Perk::FAST_RELOAD:
			FastReload();
			break;
		case Perk::FREEZE_BULLETS:
			FreezeBullets();
			break;
		}
}

void Weapon::AddPerk(Perk perk)
{
	perks.push_back(perk);
	RefreshPerks();
}

void Weapon::DamageUp()
{
	damageModifier += 1.0f;
}

void Weapon::MaxAmmoUp()
{
	maxAmmoModifier += 10;
}

void Weapon::FireRateUp()
{
	fireRateModifier -= 0.5f;
	if (fireRateModifier < 0.1f)
		fireRateModifier = 0.1f;
}

void Weapon::FastReload()
{
	reloadTime -= 0.2f;
	if (reloadTime < 0.1f)
		reloadTime = 0.1f;
}

void Weapon::FreezeBullets()
{
	onHitEffects.emplace_back(Effect(EffectType::FROZEN, 4.0f, false));
}

void Weapon::CreateProjectiles()
{
	DeleteProjectiles();

	if (projectilePrefab.uid == NULL)
		return;

	projectiles = new Projectile * [projectileNum];
	projectileHolder = App->scene->CreateGameObject("Bullets", gameObject);

	for (uint i = 0; i < projectileNum; ++i)
	{
		GameObject* object = App->resourceManager->LoadPrefab(projectilePrefab.uid, projectileHolder); // Load the prefab onto a gameobject
		Projectile* projectile = new Projectile(object);

		char n[10];
		sprintf_s(n, "%d", i);
		std::string num = n;
		std::string name("Projectile" + num);
		object->SetName(name.c_str()); // Rename it

		object->transform->SetLocalPosition(float3::zero);

		if (projectile->bulletScript)
			projectile->bulletScript->SetShooter(this, i); // Set up the bullet script

		for (uint i = 0; i < object->components.size(); ++i)
			object->components[i]->SetIsActive(false);
		object->SetIsActive(false);

		projectiles[i] = projectile;
	}
}

void Weapon::DeleteProjectiles()
{
	if (projectiles)
	{
		for (uint i = 0; i < projectileNum; ++i)
		{
			if (projectiles[i])
				delete projectiles[i];
			projectiles[i] = nullptr;
		}
		delete projectiles;
		projectiles = nullptr;
	}
	if (projectileHolder)
	{
		projectileHolder->toDelete = true;
		projectileHolder = nullptr;
	}
}

void Weapon::FireProjectile(float2 direction)
{
	if (!projectileHolder || !projectiles)
		return;

	if (direction.IsZero())
		return;
	direction.Normalize();

	Projectile* projectile = nullptr;

	for (uint i = 0; i < projectileNum; ++i)
		if (!projectiles[i]->inUse)
		{
			projectiles[i]->inUse = true;
			projectile = projectiles[i];
			break;
		}
	if (!projectile)
		return;

	float3 position = float3::zero;
	if (hand)
		position = hand->transform->GetWorldPosition();

	if (projectile->object)
	{
		projectile->object->transform->SetWorldPosition(position);

		float rad = direction.AimedAngle();
		projectile->object->transform->SetLocalRotation(float3(0, -rad, 0));

		C_RigidBody* rigidBody = projectile->object->GetComponent<C_RigidBody>();
		if (rigidBody)
		{
			float3 aimDirection = { direction.x, 0.0f, direction.y };
			rigidBody->TransformMovesRigidBody(true);
			rigidBody->SetLinearVelocity(aimDirection * ProjectileSpeed());
		}

		if (projectile->bulletScript)
			projectile->bulletScript->SetOnHitData(Damage(), onHitEffects, BulletLifeTime());

		//C_AudioSource* source = projectile->object->GetComponent<C_AudioSource>();
		//if (source)
		//	source->PlayFx(source->GetEventId());

		projectile->object->SetIsActive(true);
		for (uint i = 0; i < projectile->object->components.size(); ++i)
			projectile->object->components[i]->SetIsActive(true);
	}
}

//	// Stats
//	// Shoot
//	INSPECTOR_DRAGABLE_FLOAT(damage);
//	INSPECTOR_DRAGABLE_FLOAT(projectileSpeed);
//	INSPECTOR_DRAGABLE_FLOAT(fireRate);
//	INSPECTOR_DRAGABLE_INT(ammo);
//	INSPECTOR_DRAGABLE_INT(maxAmmo);
//	INSPECTOR_DRAGABLE_INT(projectilesPerShot);
//
//	INSPECTOR_GAMEOBJECT(hand);
//	// Reload
//	INSPECTOR_DRAGABLE_FLOAT(reloadTime);
//
//	// Modifiers
//	INSPECTOR_DRAGABLE_FLOAT(damageModifier);
//	INSPECTOR_DRAGABLE_FLOAT(projectileSpeedModifier);
//	INSPECTOR_DRAGABLE_FLOAT(fireRateModifier);
//	INSPECTOR_DRAGABLE_FLOAT(reloadTimeModifier);
//	INSPECTOR_DRAGABLE_INT(maxAmmoModifier);
//	INSPECTOR_DRAGABLE_INT(PPSModifier);
//
//	// Prefabs
//	INSPECTOR_PREFAB(weaponModel);
//	INSPECTOR_PREFAB(projectilePrefab);
//
//	// Projectiles
//	INSPECTOR_DRAGABLE_INT(projectileNum);
//	INSPECTOR_CHECKBOX_BOOL(updateProjectiles);
