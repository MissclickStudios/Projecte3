#include "Weapon.h"

#include "Application.h"
#include "M_Physics.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"

#include "MathGeoLib/include/Math/MathFunc.h"
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
			{
				shootAudio = source;
				shootAudio->SetVolume(0.5f);
			}
			else if (name == "weapon_laser_02_02")
			{
				shootAudio = source;
				shootAudio->SetVolume(0.25f);
			}
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

void Weapon::OnPause()
{
	//fireRateTimer.Pause();
	reloadTimer.Pause();

	WeaponPause();
}

void Weapon::OnResume()
{
	//fireRateTimer.Resume();
	reloadTimer.Resume();

	WeaponResume();
}

ShootState Weapon::Shoot(float2 direction)
{
	ShootState state = ShootLogic();
	if (state == ShootState::FIRED_PROJECTILE)
	{
		if (projectilesPerShot > 1 && shotSpreadArea > 0)
		{
			SpreadProjectiles(direction);
		}
		else if (projectilesPerShot != 0)
		{
			FireProjectile(direction);
		}
		
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

void Weapon::SetOwnership(EntityType type, GameObject* hand, std::string handName)
{
	this->hand = hand;

	CreateProjectiles();
	RefreshPerks();

	SetUp();

	if (this->hand != nullptr && weaponModelPrefab.uid != NULL)
	{
		GameObject* skeletonHand = GetHand(this->hand->parent, handName);
		if (skeletonHand)
			weaponModel = App->resourceManager->LoadPrefab(weaponModelPrefab.uid, skeletonHand); // Load the prefab onto a gameobject
	}

	if (type == EntityType::PLAYER)
	{
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
	maxAmmoModifier = DEFAULT_MODIFIER;
	PPSModifier = 0.0f;
	onHitEffects.clear();

	PerkType t;
	// Apply each perk
	for (uint i = 0; i < perks.size(); ++i)
		switch (perks[i].Type())
		{
		case PerkType::DAMAGE_MODIFY:
			DamageModify(&perks[i]);
			break;
		case PerkType::MAXAMMO_MODIFY:
			MaxAmmoModify(&perks[i]);
			break;
		case PerkType::FIRERATE_MODIFY:
			FireRateModify(&perks[i]);
			break;
		case PerkType::RELOAD_TIME_MODIFY:
			ReloadTimeModify(&perks[i]);
			break;
		case PerkType::BULLET_LIFETIME_MODIFY:
			BulletLifeTimeModify(&perks[i]);
			break;
		case PerkType::FREEZE_BULLETS:
			FreezeBullets(&perks[i]);
			break;
		case PerkType::STUN_BULLETS:
			StunBullets(&perks[i]);
			break;
		}
}

void Weapon::AddPerk(PerkType type, float amount, float duration)
{
	perks.push_back(Perk(type, amount, duration));
	RefreshPerks();
}

void Weapon::DamageModify(Perk* perk)
{
	damageModifier *= perk->Amount();
}

void Weapon::MaxAmmoModify(Perk* perk)
{
	maxAmmoModifier *= perk->Amount();
}

void Weapon::FireRateModify(Perk* perk)
{
	float requestedFireRate = fireRate * fireRateModifier * perk->Amount();
	if (requestedFireRate >= fireRateCap)
		fireRateModifier *= perk->Amount();
}

void Weapon::ReloadTimeModify(Perk* perk)
{
	float requestedReloadTime = reloadTime * reloadTimeModifier * perk->Amount();
	if (requestedReloadTime >= reloadTimeCap)
		reloadTimeModifier *= perk->Amount();
}

void Weapon::BulletLifeTimeModify(Perk* perk)
{
	bulletLifeTimeModifier *= perk->Amount();
}

void Weapon::FreezeBullets(Perk* perk)
{
	onHitEffects.emplace_back(Effect(EffectType::FROZEN, perk->Duration(), false, nullptr));
}

void Weapon::StunBullets(Perk* perk)
{
	onHitEffects.emplace_back(Effect(EffectType::STUN, perk->Duration(), false, new std::pair<bool, float>(true, perk->Amount())));
}

void Weapon::SpreadProjectiles(float2 direction)
{
	float sin = math::Sin(DegToRad(shotSpreadArea));

	float cos = math::Cos(DegToRad(shotSpreadArea));

	float2 initialDirection = direction;
	for (uint i = 0; i < projectilesPerShot; ++i)
	{
		FireProjectile(direction);

		if (i < projectilesPerShot / 2)
		{
			direction = float2(cos * direction.x + (-sin * direction.y), sin * direction.x + (cos * direction.y));
		}
		else if (i == int(projectilesPerShot / 2))
		{
			direction = initialDirection;
			sin = math::Sin(DegToRad(-shotSpreadArea));
			cos = math::Cos(DegToRad(-shotSpreadArea));

			direction = float2(cos * direction.x + (-sin * direction.y), sin * direction.x + (cos * direction.y));

		}
		else if (i >= projectilesPerShot / 2)
		{
			direction = float2(cos * direction.x + (-sin * direction.y), sin * direction.x + (cos * direction.y));
		}
	}
}

GameObject* Weapon::GetHand(GameObject* object, std::string handName)
{
	for (int i = 0; i < object->childs.size(); ++i)
	{
		std::string name = object->childs[i]->GetName();
		bool check = (name == handName);
		if (check)
			return object->childs[i];

		GameObject* output = GetHand(object->childs[i], handName);
		if (output != nullptr)
			return output;
	}
	return nullptr;
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
