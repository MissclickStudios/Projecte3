#include "SandTrooper.h"
#include "Random.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"
#include "C_NavMeshAgent.h"

#include "MathGeoLib/include/Math/float2.h"

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

	INSPECTOR_STRING(script->rightHandName);

	INSPECTOR_SLIDER_INT(script->minCredits, 0, 1000);
	INSPECTOR_SLIDER_INT(script->maxCredits, 0, 1000);

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

	agent = gameObject->GetComponent<C_NavMeshAgent>();

	if (agent != nullptr)
	{
		agent->origin = gameObject->GetComponent<C_Transform>()->GetWorldPosition();
		agent->velocity = ChaseSpeed();
	}

	//Audios
	damageAudio = new C_AudioSource(gameObject);
	deathAudio = new C_AudioSource(gameObject);
	walkAudio = new C_AudioSource(gameObject);
	if (damageAudio != nullptr)
		damageAudio->SetEvent("sandtrooper_damaged");
	if (deathAudio != nullptr)
		deathAudio->SetEvent("sandtrooper_death");		
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

	if (damageAudio != nullptr)
		delete damageAudio;
	if (deathAudio != nullptr)
		delete deathAudio;
	if (walkAudio != nullptr)
		delete walkAudio;
}

void Trooper::EntityPause()
{
	if (agent != nullptr)
		agent->CancelDestination();
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
}

void Trooper::LookAtPlayer()
{
	float2 direction = aimDirection;
	if (moveState != TrooperState::IDLE)
		direction = moveDirection;
	float rad = direction.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void Trooper::ManageMovement()
{
	if (dieAfterStun == 2)
	{
		dieAfterStun = 3;
		moveState = TrooperState::DEAD_IN;
		deathTimer.Resume();
	}
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
		currentAnimation = &idleAnimation;
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
		if (distance > chaseDistance)
		{
			moveState = TrooperState::PATROL;
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
			if (dieAfterStun == 1)
				playerScript->GiveCredits(Random::LCG::GetBoundedRandomUint(minCredits, maxCredits));
		}
		deathTimer.Start();
		moveState = TrooperState::DEAD;

	case TrooperState::DEAD:
		if (dieAfterStun > 1)
			deathTimer.Resume();
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
			case ShootState::WAITING_FOR_NEXT:
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
		currentAnimation = &idleAnimation;
		aimState = AimState::RELOAD;

	case AimState::RELOAD:
		currentAnimation = &idleAnimation;
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
	if (agent != nullptr)
		agent->StopAndCancelDestination();
}

void Trooper::Chase()
{
	if (agent != nullptr)
	{
		agent->SetDestination(player->transform->GetWorldPosition());
		moveDirection = float2(agent->direction.x, agent->direction.z);

		runAnimation.duration = 2 / speedModifier;
	}
}

void Trooper::Flee()
{
	if (agent != nullptr)
		agent->SetDestination(-player->transform->GetWorldPosition());
}

