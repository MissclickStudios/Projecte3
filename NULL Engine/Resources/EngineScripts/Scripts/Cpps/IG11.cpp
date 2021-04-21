#include "IG11.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Player.h"

IG11* CreateIG11()
{
	IG11* script = new IG11();

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
	INSPECTOR_STRING(script->playerName);

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


IG11::IG11() : Entity()
{
	type = EntityType::IG11;
}

IG11::~IG11()
{
}

void IG11::SetUp()
{
	player = App->scene->GetGameObjectByName(playerName.c_str());

	GameObject* hand = nullptr;
	if (skeleton)
	{
		for (uint i = 0; i < skeleton->childs.size(); ++i)
		{
			std::string name = skeleton->childs[i]->GetName();
			if (name == "Hand")
			{
				hand = skeleton->childs[i];
				break;
			}
		}
	}
	

	// Create Weapons and save the Weapon script pointer
	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());

	blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	blasterWeapon->SetOwnership(type, hand);
}

void IG11::Update()
{
	ManageMovement();
	if (moveState != IG11State::DEAD)
		ManageAim();
}

void IG11::CleanUp()
{
	blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;
}

void IG11::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void IG11::DistanceToPlayer()
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

void IG11::LookAtPlayer()
{
	float rad = aimDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void IG11::ManageMovement()
{
	if (moveState != IG11State::DEAD)
	{
		if (health <= 0.0f)
			moveState = IG11State::DEAD_IN;
		else
		{
			DistanceToPlayer();
			if (moveState != IG11State::PATROL)
				LookAtPlayer();
		}
	}

	switch (moveState)
	{
	case IG11State::IDLE:
		rigidBody->SetLinearVelocity(float3::zero);
		if (aimState == AimState::SHOOT) // Comented till the shooting system is in place
			break;
		if (distance > chaseDistance)
		{
			moveState = IG11State::PATROL;
			break;
		}
		if (distance > attackDistance)
		{
			moveState = IG11State::CHASE;
			break;
		}
		if (distance < fleeDistance)
		{
			moveState = IG11State::FLEE;
			break;
		}
		break;
	case IG11State::PATROL:
		currentAnimation = &walkAnimation;
		if (distance < chaseDistance)
		{
			moveState = IG11State::CHASE;
			break;
		}
		Patrol();
		break;
	case IG11State::CHASE:
		currentAnimation = &runAnimation;
		if (distance < attackDistance)
		{
			moveState = IG11State::IDLE;
			break;
		}
		Chase();
		break;
	case IG11State::FLEE:
		currentAnimation = &fleeAnimation;
		if (distance > fleeDistance)
		{
			moveState = IG11State::IDLE;
			break;
		}
		Flee();
		break;
	case IG11State::DEAD_IN:
		currentAnimation = &deathAnimation;
		deathTimer.Start();
		moveState = IG11State::DEAD;

	case IG11State::DEAD:
		if (deathTimer.ReadSec() >= deathDuration)
			Deactivate();
		break;
	}
}

void IG11::ManageAim()
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

void IG11::Patrol()
{
	rigidBody->SetLinearVelocity(float3::zero);
}

void IG11::Chase()
{
	float3 direction = { moveDirection.x, 0.0f, moveDirection.y };
	rigidBody->SetLinearVelocity(direction * ChaseSpeed());
}

void IG11::Flee()
{
	float3 direction = { -moveDirection.x, 0.0f, -moveDirection.y };
	rigidBody->SetLinearVelocity(direction * ChaseSpeed());
}
