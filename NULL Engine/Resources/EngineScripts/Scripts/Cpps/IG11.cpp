#include "IG11.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Player.h"

#include "MC_Time.h"

IG11* CreateIG11()
{
	IG11* script = new IG11();

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

	//IG-11
	// Chase
	INSPECTOR_DRAGABLE_FLOAT(script->chaseDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chaseSpeedModifier);

	// Flee
	INSPECTOR_DRAGABLE_FLOAT(script->fleeDistance);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->attackDistance);

	// Special Attack
	INSPECTOR_DRAGABLE_FLOAT(script->specialAttackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->specialAttackSpins);
	INSPECTOR_DRAGABLE_FLOAT(script->specialAttackHp);
	INSPECTOR_DRAGABLE_FLOAT(script->specialAttackCooldown);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);
	INSPECTOR_PREFAB(script->sniper);

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
	specialAttackTimer.Stop();

	player = App->scene->GetGameObjectByName(playerName.c_str());

	GameObject* handLeft = nullptr;
	GameObject* handRight = nullptr;
	if (skeleton)
	{
		for (uint i = 0; i < skeleton->childs.size(); ++i)
		{
			std::string name = skeleton->childs[i]->GetName();
			if (name == "HandLeft")
			{
				handLeft = skeleton->childs[i];
				continue;
			}
			if (name == "HandRight")
			{
				handRight = skeleton->childs[i];
				continue;
			}

		}
	}
	
	// Create Weapons and save the Weapon script pointer
	if (blaster.uid != NULL)
		blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());

	if (blasterGameObject)
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	if (blasterWeapon)
		blasterWeapon->SetOwnership(type, handLeft);

	if (sniper.uid != NULL)
		sniperGameObject = App->resourceManager->LoadPrefab(sniper.uid, App->scene->GetSceneRoot());

	if (sniperGameObject)
		sniperWeapon = (Weapon*)GetObjectScript(sniperGameObject, ObjectType::WEAPON);
	if (sniperWeapon)
		sniperWeapon->SetOwnership(type, handRight);
}

void IG11::Update()
{
	ManageMovement();
	if (moveState != IG11State::DEAD)
		ManageAim();

}

void IG11::CleanUp()
{
	if (blasterGameObject)
		blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;

	if (sniperGameObject)
		sniperGameObject->toDelete = true;
	sniperGameObject = nullptr;
	sniperWeapon = nullptr;
}

void IG11::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void IG11::DistanceToPlayer()
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

	if (specialAttackTimer.IsActive() && specialAttackTimer.ReadSec() >= specialAttackCooldown)
	{
		specialAttackTimer.Stop();
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
		if (health <= 5.0f && !specialAttackTimer.IsActive())
		{
			specialAttackTimer.Start();
			moveState = IG11State::SPECIAL_ATTACK_IN;
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
	case IG11State::SPECIAL_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;

		if (blasterWeapon)
		{
			blasterWeapon->fireRate = 0.05f;
			blasterWeapon->ammo = 200;
		}
		if (sniperWeapon)
		{
			sniperWeapon->fireRate = 0.05f;
			sniperWeapon->ammo = 200;
		}

		moveState = IG11State::SPECIAL_ATTACK;
		
	case IG11State::SPECIAL_ATTACK:
		if (!SpecialAttack())
		{
			moveState = IG11State::IDLE;

			if (blasterWeapon)
			{
				blasterWeapon->fireRate = 0.3f;
				blasterWeapon->ammo = 0;
			}
			if (sniperWeapon)
			{
				sniperWeapon->fireRate = 0.3f;
				sniperWeapon->ammo = 0;
			}
		}
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
		if (distance < attackDistance || moveState == IG11State::SPECIAL_ATTACK)
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
		switch (sniperWeapon->Shoot(aimDirection))
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
		currentAnimation = &reloadAnimation;
	case AimState::RELOAD:
		if (blasterWeapon->Reload())
			aimState = AimState::ON_GUARD;
		if (sniperWeapon->Reload())
			aimState = AimState::ON_GUARD;
		break;
	case AimState::CHANGE_IN:
		aimState = AimState::CHANGE;

	case AimState::CHANGE:
		aimState = AimState::ON_GUARD;
		break;
	}

	if(moveState == IG11State::SPECIAL_ATTACK) currentAnimation = &specialAnimation;
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

bool IG11::SpecialAttack()
{
	specialAttackRot += specialAttackSpeed * MC_Time::Game::GetDT();

	float angle = specialAttackStartAim.AimedAngle();
	angle += DegToRad(specialAttackRot);

	float x = cos(angle);
	float y = sin(angle);
	aimDirection = { x,y };

	if (specialAttackRot >= 360.0f * specialAttackSpins)
		return false;
	
	return true;
}
