#include "SandTrooper.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"

#include "Player.h"

Trooper* CreateTrooper()
{
	Trooper* script = new Trooper();
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

	// Trooper ---
	// Chase
	INSPECTOR_DRAGABLE_FLOAT(script->chaseDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chaseSpeedModifier);

	// Flee
	INSPECTOR_DRAGABLE_FLOAT(script->fleeDistance);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->attackDistance);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);

	//Hand Name

	INSPECTOR_STRING(script->handName);

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
	if(blaster.uid != NULL)
		blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());

	if (blasterGameObject)
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	if (blasterWeapon)
		blasterWeapon->SetOwnership(type, hand);

	for (uint i = 0; i < gameObject->components.size(); ++i)
	{
		if (gameObject->components[i]->GetType() == ComponentType::AUDIOSOURCE)
		{
			C_AudioSource* source = (C_AudioSource*)gameObject->components[i];
			std::string name = source->GetEventName();

			if (name == "sandstormtrooper_walking")
				walkAudio = source;
			else if (name == "sandstormtrooper_idle")
				damageAudio = source;
			else if (name == "sandstormtrooper_death")
				deathAudio = source;
		}
	}
}

void Trooper::Behavior()
{
	ManageMovement();
	if (moveState != TrooperState::DEAD)
		ManageAim();
}

void Trooper::CleanUp()
{
	if (blasterGameObject)
		blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;
}

void Trooper::EntityPause()
{
}

void Trooper::EntityResume()
{
}

void Trooper::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void Trooper::DistanceToPlayer()
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
		if (rigidBody != nullptr)
			rigidBody->Set2DVelocity(float2::zero);
		if (aimState == AimState::SHOOT) // Prioritize shooting over moving
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
		if (blasterWeapon)
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
		if (blasterWeapon && blasterWeapon->Reload())
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
	if (rigidBody != nullptr)
		rigidBody->SetLinearVelocity(float3::zero);
}

void Trooper::Chase()
{
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(moveDirection * ChaseSpeed());
}

void Trooper::Flee()
{
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(-moveDirection * ChaseSpeed());
}