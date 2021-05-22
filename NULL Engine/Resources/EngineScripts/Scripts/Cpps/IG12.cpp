#include "IG12.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

#include "Player.h"

#include "MC_Time.h"

IG12* CreateIG12()
{
	IG12* script = new IG12();

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

	//IG-12
	// Chase
	INSPECTOR_DRAGABLE_FLOAT(script->chaseDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chaseSpeedModifier);

	// Flee
	INSPECTOR_DRAGABLE_FLOAT(script->fleeDistance);

	// Attack
	INSPECTOR_DRAGABLE_FLOAT(script->attackDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->firstStageAttackCooldown);
	INSPECTOR_DRAGABLE_FLOAT(script->secondStageAttackCooldown);


	// Spiral Attack
	INSPECTOR_DRAGABLE_FLOAT(script->spiralAttackDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->spiralAttackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->spiralAttackSpins);
	INSPECTOR_DRAGABLE_FLOAT(script->spiralAttackHp);

	// Line Attack
	INSPECTOR_DRAGABLE_FLOAT(script->lineAttackDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->lineAttackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->lineAttackShots);
	INSPECTOR_DRAGABLE_FLOAT(script->lineAttackSpins);
	INSPECTOR_DRAGABLE_FLOAT(script->lineAttackHp);

	// Bombing Attack
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackShots);
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackHp);
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackBigAreaSide);
	INSPECTOR_DRAGABLE_FLOAT(script->bombingAttackSmallAreaSide);
	INSPECTOR_DRAGABLE_FLOAT(script->bombFallingTime);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);
	INSPECTOR_PREFAB(script->sniper);
	
	//Hand Name
	INSPECTOR_STRING(script->rightHandName);
	INSPECTOR_STRING(script->leftHandName);

	return script;
}

IG12::IG12() : Entity()
{
	type = EntityType::IG12;
}

IG12::~IG12()
{
}

void IG12::SetUp()
{
	firstStageTimer.Stop();
	secondStageTimer.Stop();

	player = App->scene->GetGameObjectByName(playerName.c_str());
	userAttackDistance = attackDistance;

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
		blasterWeapon->SetOwnership(type, handLeft, leftHandName);

	if (sniper.uid != NULL)
		sniperGameObject = App->resourceManager->LoadPrefab(sniper.uid, App->scene->GetSceneRoot());

	if (sniperGameObject)
		sniperWeapon = (Weapon*)GetObjectScript(sniperGameObject, ObjectType::WEAPON);
	if (sniperWeapon)
		sniperWeapon->SetOwnership(type, handRight, rightHandName);
	
	crosshair = gameObject->FindChild("Aim");
}

void IG12::Behavior()
{
	ManageMovement();
	if (moveState != IG12State::DEAD)
		ManageAim();
}

void IG12::CleanUp()
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

void IG12::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(Damage());
}

void IG12::DistanceToPlayer()
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

void IG12::LookAtPlayer()
{
	float rad = aimDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void IG12::ManageMovement()
{
	if (moveState != IG12State::DEAD)
	{
		if (health <= 0.0f)
			moveState = IG12State::DEAD_IN;
		else
		{
			DistanceToPlayer();
			if (moveState != IG12State::PATROL)
				LookAtPlayer();
		}
	}
	if (firstStageTimer.IsActive() && firstStageTimer.ReadSec() >= firstStageAttackCooldown)
	{
		firstStageTimer.Stop();
		pickFirstStageAttack();
	}
	else if (secondStageTimer.IsActive() && secondStageTimer.ReadSec() >= secondStageAttackCooldown)
	{
		secondStageTimer.Stop();
		pickSecondStageAttack();
	}
	else if (spiralAttackTimer.IsActive() && spiralAttackTimer.ReadSec() >= spiralAttackDuration)
	{
		spiralAttackTimer.Stop();
		if (health <= 5)
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}	
	else if (lineAttackTimer.IsActive() && lineAttackTimer.ReadSec() >= lineAttackDuration)
	{
		lineAttackTimer.Stop();
		lineAttackShots += 15;
		if (health <= 5)
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}
	else if (bombingAttackTimer.IsActive() && bombingAttackTimer.ReadSec() >= bombingAttackDuration)
	{
		bombingAttackTimer.Stop();
		bombingAttackShots = 5;
		if (health <= 5)
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}
	else if (!firstStageTimer.IsActive() && !secondStageTimer.IsActive())
	{
		if (health <= 5)
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}

	switch (moveState)
	{
	case IG12State::IDLE:
		if (rigidBody != nullptr)
			rigidBody->Set2DVelocity(float2::zero);
		if (aimState == AimState::SHOOT)
			break;
		if (distance > chaseDistance)
		{
			moveState = IG12State::PATROL;
			break;
		}
		if (distance > attackDistance)
		{
			moveState = IG12State::CHASE;
			break;
		}
		if (distance < fleeDistance)
		{
			moveState = IG12State::FLEE;
			break;
		}

		break;
	case IG12State::PATROL:
		currentAnimation = &walkAnimation;
		if (distance < chaseDistance)
		{
			moveState = IG12State::CHASE;
			break;
		}
		Patrol();
		break;
	case IG12State::CHASE:
		currentAnimation = &runAnimation;
		if (distance < attackDistance)
		{
			moveState = IG12State::IDLE;
			break;
		}
		Chase();
		break;
	case IG12State::FLEE:
		currentAnimation = &fleeAnimation;
		if (distance > fleeDistance)
		{
			moveState = IG12State::IDLE;
			break;
		}
		Flee();
		break;
	case IG12State::LINE_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		attackDistance = 200.0f;
		moveState = IG12State::LINE_ATTACK;

	case IG12State::LINE_ATTACK:
		if (!LineAttack())
		{
			moveState = IG12State::IDLE;
			attackDistance = userAttackDistance;
			if (health > 5.0f)
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 1;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 1;
				}
			}
			else
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 2;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 2;
				}
			}
		}
		break;
	case IG12State::BOMBING_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		attackDistance = 200.0f;
		moveState = IG12State::BOMBING_ATTACK;

	case IG12State::BOMBING_ATTACK:
		if (!BombingAttack())
		{
			moveState = IG12State::IDLE;
			attackDistance = userAttackDistance;

			if (health > 5.0f)
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 1;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 1;
				}
			}
			else
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 2;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 2;
				}
			}
		}
		break;
	case IG12State::SPIRAL_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		
		moveState = IG12State::SPIRAL_ATTACK;

	case IG12State::SPIRAL_ATTACK:
		if (!SpiralAttack())
		{
			moveState = IG12State::IDLE;

			if (health > 5.0f)
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 1;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 1;
				}
			}
			else
			{
				if (blasterWeapon)
				{
					blasterWeapon->fireRate = 0.3f;
					blasterWeapon->ammo = 0;
					blasterWeapon->projectilesPerShot = 2;
				}
				if (sniperWeapon)
				{
					sniperWeapon->fireRate = 0.3f;
					sniperWeapon->ammo = 0;
					sniperWeapon->projectilesPerShot = 2;
				}
			}
		}
		break;    
	case IG12State::DEAD_IN:
		currentAnimation = &deathAnimation;
		deathTimer.Start();
		moveState = IG12State::DEAD;

	case IG12State::DEAD:
		if (deathTimer.ReadSec() >= deathDuration)
			Deactivate();
		break;
	}
}

void IG12::ManageAim()
{
	switch (aimState)
	{
	case AimState::IDLE:
		if (distance < attackDistance || moveState == IG12State::SPIRAL_ATTACK)
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

		if (moveState != IG12State::SPIRAL_ATTACK)
			secondaryAimDirection = aimDirection;
		if (moveState == IG12State::LINE_ATTACK)
			lineAttackShots--;

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
		switch (sniperWeapon->Shoot(secondaryAimDirection))
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

	if (moveState == IG12State::SPIRAL_ATTACK) currentAnimation = &specialAnimation;
}

void IG12::Patrol()
{
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(float2::zero);
}

void IG12::Chase()
{
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(moveDirection * ChaseSpeed());
}

void IG12::Flee()
{
	if (rigidBody != nullptr)
		rigidBody->Set2DVelocity(-moveDirection * ChaseSpeed());
}

bool IG12::SpiralAttack()
{
	specialAttackRot += spiralAttackSpeed * MC_Time::Game::GetDT();
	float angle = specialAttackStartAim.AimedAngle();
	angle += DegToRad(specialAttackRot);

	float x = cos(angle);
	float y = sin(angle);
	aimDirection = { x,y };

	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.0001f;
		blasterWeapon->ammo = 20;
		blasterWeapon->projectilesPerShot = 3;
		blasterWeapon->shotSpreadArea = 5;
	}
	if (sniperWeapon)
	{
		sniperWeapon->fireRate = 0.01f;
		sniperWeapon->ammo = 20;
		sniperWeapon->projectilesPerShot = 3;
		sniperWeapon->shotSpreadArea = 5;
		secondaryAimDirection = -aimDirection;
	}

	/*if (specialAttackRot >= 360.0f * spiralAttackSpins)
		return false;*/
	if (!spiralAttackTimer.IsActive())
		return false;

	return true;
}

bool IG12::LineAttack()
{
	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.001f;
		blasterWeapon->projectilesPerShot = 3;
		blasterWeapon->ammo = 30;
		blasterWeapon->shotSpreadArea = 3;
	}

	if (sniperWeapon)
		sniperWeapon->projectilesPerShot = 0;
	
	if (!lineAttackTimer.IsActive())
		return false;

	return true;
}

bool IG12::BombingAttack()
{
	if (blasterWeapon)
	{
		blasterWeapon->projectilesPerShot = 0;
	}
	if (sniperWeapon)
	{
		sniperWeapon->projectilesPerShot = 0;	
	}

	if (bombTimer.IsActive() && bombTimer.ReadSec() >= bombFallingTime)
	{
		bombTimer.Stop();

		if(playerPosition.x >= bombPosition.x - bombingAttackSmallAreaSide && playerPosition.x < bombPosition.x + bombingAttackSmallAreaSide)
			if (playerPosition.y >= bombPosition.y - bombingAttackSmallAreaSide && playerPosition.y < bombPosition.y + bombingAttackSmallAreaSide)
			{
				LOG("Player bomb hit");
			}	
	}
	else if (!bombTimer.IsActive())
	{
		playerPosition.x = player->transform->GetWorldPosition().x;
		playerPosition.y = player->transform->GetWorldPosition().z;
		bombTimer.Start();
		bombPosition = CalculateNextBomb(playerPosition.x, playerPosition.y);
		crosshair->transform->SetWorldPosition(float3(bombPosition.x, 5, bombPosition.y));
	}

	if (!bombingAttackTimer.IsActive())
		return false;

	return true;
}

float2 IG12::CalculateNextBomb(float x, float y)
{
	float bombX = Lerp(x - (bombingAttackBigAreaSide * 0.5), x + (bombingAttackBigAreaSide * 0.5), randomGenerator.Float());
	float bombY = Lerp(y - (bombingAttackBigAreaSide * 0.5), y + (bombingAttackBigAreaSide * 0.5), randomGenerator.Float());
	return float2(bombX, bombY);
}

void IG12::pickFirstStageAttack()
{
	//bombing attack, straight line attack
	float randAttack = Lerp(0, 2, randomGenerator.Float());

	if (randAttack <= 1)
	{
		bombingAttackTimer.Start();
		moveState = IG12State::BOMBING_ATTACK_IN;
	}
	else
	{
		lineAttackTimer.Start();
		moveState = IG12State::LINE_ATTACK_IN;
	}
}

void IG12::pickSecondStageAttack()
{
	//spiral attack, straight line attack
	float randAttack = Lerp(0, 2, randomGenerator.Float());

	if (randAttack <= 1)
	{
		bombingAttackTimer.Start();
		moveState = IG12State::BOMBING_ATTACK_IN;
	}
	else
	{
		spiralAttackTimer.Start();
		moveState = IG12State::SPIRAL_ATTACK_IN;
	}
}



