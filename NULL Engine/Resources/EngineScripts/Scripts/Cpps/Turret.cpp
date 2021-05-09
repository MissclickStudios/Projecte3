#include "Turret.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"

#include "Player.h"

SCRIPTS_FUNCTION Turret* CreateTurret()
{
	Turret* script = new Turret();
	INSPECTOR_STRING(script->playerName);
	// Entity ---
	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->defense);

	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealthModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->cooldownModifier);

	// Death
	INSPECTOR_DRAGABLE_FLOAT(script->deathDuration);

	// Turret ---
	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->attackDistance);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);

	//Hand Name

	INSPECTOR_STRING(script->handName);

	return script;
}

Turret::Turret() : Entity()
{
	type = EntityType::TURRET;
}

Turret::~Turret()
{
}

void Turret::SetUp()
{
	if (rigidBody != nullptr)
		rigidBody->SetKinematic(false);

	player = App->scene->GetGameObjectByName(playerName.c_str());

	GameObject* hand = nullptr;
	if (skeleton)
		for (uint i = 0; i < skeleton->childs.size(); ++i)
		{
			std::string name = skeleton->childs[i]->GetName();
			if (name == "Hand")
			{
				hand = skeleton->childs[i];
				break;
			}
		}

	// Create Weapons and save the Weapon script pointer
	if (blaster.uid != NULL)
		blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());

	if (blasterGameObject)
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	if (blasterWeapon)
		blasterWeapon->SetOwnership(type, hand);
}

void Turret::Behavior()
{
	if (moveState != TurretState::DEAD)
	{
		if (health <= 0.0f)
			moveState = TurretState::DEAD_IN;
		else
		{
			DistanceToPlayer();
			if (moveState == TurretState::ATTACK)
				LookAtPlayer();
		}
	}

	switch (moveState)
	{
	case TurretState::IDLE:
		if (distance < attackDistance)
		{
			moveState = TurretState::ATTACK;
			break;
		}
	
		break;
	case TurretState::ATTACK:
		if (distance > attackDistance)
		{
			moveState = TurretState::IDLE;
			break;
		}
		break;
	case TurretState::DEAD_IN:
		if (deathAudio)
			deathAudio->PlayFx(deathAudio->GetEventId());

		currentAnimation = &deathAnimation;
		if (rigidBody != nullptr)
			rigidBody->SetIsActive(false); // Disable the rigidbody to avoid more interactions with other entities
		if (player)
		{
			Player* playerScript = (Player*)player->GetScript("Player");
			playerScript->currency += 50;
		}
		deathTimer.Start();
		moveState = TurretState::DEAD;

	case TurretState::DEAD:
		if (deathTimer.ReadSec() >= deathDuration)
			Deactivate();
		break;
	}
	if (moveState != TurretState::DEAD)
		ManageAim();
}

void Turret::CleanUp()
{
	if (blasterGameObject)
		blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;
}

void Turret::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void Turret::DistanceToPlayer()
{
	if (!player)
		return;
	float2 playerPosition, position;
	playerPosition.x = player->transform->GetWorldPosition().x;
	playerPosition.y = player->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;
	aimDirection = playerPosition - position;

	distance = aimDirection.Length();
}

void Turret::LookAtPlayer()
{
	float rad = aimDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void Turret::ManageAim()
{
	switch (aimState)
	{
	case AimState::IDLE:
		if (distance < attackDistance)
			aimState = AimState::SHOOT_IN;
		break;
	case AimState::SHOOT_IN:
		currentAnimation = &shootAnimation;
		aimState = AimState::SHOOT;

	case AimState::SHOOT:
		currentAnimation = &shootAnimation; // temporary till torso gets an independent animator
		if (blasterWeapon)
			switch (blasterWeapon->Shoot(aimDirection))
			{
			case ShootState::NO_FULLAUTO:
				currentAnimation = nullptr;
				aimState = AimState::IDLE;
				break;
			case ShootState::WAINTING_FOR_NEXT:
				break;
			case ShootState::FIRED_PROJECTILE:
				currentAnimation = nullptr;
				aimState = AimState::IDLE;
				break;
			case ShootState::RATE_FINISHED:
				currentAnimation = nullptr;
				aimState = AimState::IDLE;
				break;
			case ShootState::NO_AMMO:
				aimState = AimState::RELOAD_IN;
				break;
			}
		break;
	case AimState::RELOAD_IN:
		aimState = AimState::RELOAD;

	case AimState::RELOAD:
		if (blasterWeapon && blasterWeapon->Reload())
			aimState = AimState::IDLE;
		break;
	case AimState::CHANGE_IN:
		aimState = AimState::CHANGE;

	case AimState::CHANGE:
		aimState = AimState::IDLE;
		break;
	}
}