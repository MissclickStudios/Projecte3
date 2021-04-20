#include "SandTrooper.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Player.h"

SCRIPTS_FUNCTION Trooper* CreateTrooper()
{
	Trooper* script = new Trooper();

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

	// Trooper ---
	// Movement
	INSPECTOR_GAMEOBJECT(script->player);

	// Chase
	INSPECTOR_DRAGABLE_FLOAT(script->chaseDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chaseSpeedModifier);

	// Flee
	INSPECTOR_DRAGABLE_FLOAT(script->fleeDistance);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->attackDistance);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);

	return script;
}


Trooper::Trooper() : Entity()
{
	type = EntityType::TROOPER;
}

Trooper::~Trooper()
{
}

void Trooper::SetUp()
{
	GameObject* hand = nullptr;
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
	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());

	blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	blasterWeapon->SetOwnership(type, hand);
}

void Trooper::Update()
{
	ManageMovement();
	if (moveState != TrooperState::DEAD)
		ManageAim();
}

void Trooper::CleanUp()
{
	blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;
}

void Trooper::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void Trooper::DistanceToPlayer()
{
	float2 playerPosition, position;
	playerPosition.x = player->transform->GetWorldPosition().x;
	playerPosition.y = player->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;
	aimDirection = playerPosition - position;

	distance = aimDirection.Length();
	// TODO: Separate aim and movement once the pathfinding is implemented
	moveDirection = aimDirection;

	if (!moveDirection.IsZero())
		moveDirection.Normalize();
}

void Trooper::LookAtPlayer()
{
	float rad = aimDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void Trooper::ManageMovement()
{
	if (moveState != TrooperState::DEAD)
	{
		if (health <= 0.0f)
			moveState = TrooperState::DEAD_IN;
		else
		{
			DistanceToPlayer();
			if (moveState != TrooperState::PATROL)
				LookAtPlayer();
		}
	}

	switch (moveState)
	{
	case TrooperState::IDLE:
		rigidBody->SetLinearVelocity(float3::zero);
		if (aimState == AimState::SHOOT) // Comented till the shooting system is in place
			break;
		if (distance > chaseDistance)
		{
			moveState = TrooperState::PATROL;
			break;
		}
		if (distance > attackDistance)
		{
			moveState = TrooperState::CHASE;
			break;
		}
		if (distance < fleeDistance)
		{
			moveState = TrooperState::FLEE;
			break;
		}
		break;
	case TrooperState::PATROL:
		currentAnimation = &walkAnimation;
		if (distance < chaseDistance)
		{
			moveState = TrooperState::CHASE;
			break;
		}
		Patrol();
		break;
	case TrooperState::CHASE:
		currentAnimation = &runAnimation;
		if (distance < attackDistance)
		{
			moveState = TrooperState::IDLE;
			break;
		}
		Chase();
		break;
	case TrooperState::FLEE:
		currentAnimation = &fleeAnimation;
		if (distance > fleeDistance)
		{
			moveState = TrooperState::IDLE;
			break;
		}
		Flee();
		break;
	case TrooperState::DEAD_IN:
		currentAnimation = &deathAnimation;
		deathTimer.Start();
		moveState = TrooperState::DEAD;

	case TrooperState::DEAD:
		if (deathTimer.ReadSec() >= deathDuration)
			Deactivate();
		break;
	}
}

void Trooper::ManageAim()
{
	switch (aimState)
	{
	case AimState::IDLE:
		if (distance < attackDistance)
			aimState = AimState::SHOOT_IN;
		break;
	case AimState::ON_GUARD:
		aimState = AimState::IDLE;
		break;
	case AimState::SHOOT_IN:
		currentAnimation = &shootAnimation;
		aimState = AimState::SHOOT;

	case AimState::SHOOT:
		currentAnimation = &shootAnimation; // temporary till torso gets an independent animator
		switch (blasterWeapon->Shoot(aimDirection))
		{
		case ShootState::NO_FULLAUTO:
			currentAnimation = nullptr;
		aimState = AimState::ON_GUARD;
			break;
		case ShootState::WAINTING_FOR_NEXT:
			break;
		case ShootState::FIRED_PROJECTILE:
			currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::RATE_FINISHED:
			currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::NO_AMMO:
			aimState = AimState::RELOAD_IN;
			break;
		}
		break;
	case AimState::RELOAD_IN:
		aimState = AimState::RELOAD;

	case AimState::RELOAD:
		if (blasterWeapon->Reload())
			aimState = AimState::ON_GUARD;
		break;
	case AimState::CHANGE_IN:
		aimState = AimState::CHANGE;

	case AimState::CHANGE:
		aimState = AimState::ON_GUARD;
		break;
	}
}

void Trooper::Patrol()
{
	rigidBody->SetLinearVelocity(float3::zero);
}

void Trooper::Chase()
{
	float3 direction = { moveDirection.x, 0.0f, moveDirection.y };
	rigidBody->SetLinearVelocity(direction * ChaseSpeed());
}

void Trooper::Flee()
{
	float3 direction = { -moveDirection.x, 0.0f, -moveDirection.y };
	rigidBody->SetLinearVelocity(direction * ChaseSpeed());
}