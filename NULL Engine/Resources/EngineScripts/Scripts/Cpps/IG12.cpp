#include "IG12.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_ParticleSystem.h"
#include "C_BoxCollider.h"

#include "Player.h"
#include "GameManager.h"
#include "DialogManager.h"
#include "CameraMovement.h"

#include "MC_Time.h"

#include "C_AudioSource.h"
#include "C_Canvas.h"
#include "C_UI_Image.h"
#include "M_UISystem.h"

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
	INSPECTOR_DRAGABLE_FLOAT(script->prepareBombingTime);

	//Weapons
	INSPECTOR_PREFAB(script->blaster);
	INSPECTOR_PREFAB(script->sniper);
	INSPECTOR_PREFAB(script->bomb);
	INSPECTOR_PREFAB(script->bombProjectile);
	//Hand Name
	INSPECTOR_STRING(script->rightHandName);
	INSPECTOR_STRING(script->leftHandName);

	//Health Bar
	INSPECTOR_GAMEOBJECT(script->healthBarCanvasObject);
	INSPECTOR_STRING(script->lifeBarImageStr);
	INSPECTOR_STRING(script->bossIconStr);

	//Beskar
	INSPECTOR_INPUT_INT(script->beskarValue);

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
	bombTimer.Stop();
	prepareBombing.Stop();

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
	//Get Bomb prefab with crosshair and projectile
	if (bomb.uid != NULL)
	{
		bombGameObject = App->scene->InstantiatePrefab(bomb.uid, App->scene->GetSceneRoot(), float3::zero, Quat());
	}
	if (bombGameObject != nullptr)
	{
		bombCollider = bombGameObject->GetComponent<C_BoxCollider>();
		if (bombCollider != nullptr)
			bombCollider->SetIsActive(false);
		
		crosshair = bombGameObject->FindChild("Aim");
	}

	crosshair->SetIsActive(false);

	if(bombProjectile.uid != NULL)
		projectileGameObject = App->scene->InstantiatePrefab(bombProjectile.uid, App->scene->GetSceneRoot(), float3::zero, Quat());

	if (projectileGameObject)
		projectileGameObject->SetIsActive(false);

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
	
	//GameManager
	GameObject* tmp = App->scene->GetGameObjectByName("Game Manager");

	if (tmp != nullptr)
	{
		gameManager = (GameManager*)tmp->GetScript("GameManager");
	}


	if (healthBarCanvasObject)
	{
		healthBarCanvas = healthBarCanvasObject->GetComponent<C_Canvas>();
		if (healthBarCanvas)
			App->uiSystem->PushCanvas(healthBarCanvas);
		for (int i = 0; i < healthBarCanvasObject->childs.size(); ++i)
		{
			if (!strcmp(healthBarCanvasObject->childs[i]->GetName(), lifeBarImageStr.c_str()))
			{

				healthBarImage = healthBarCanvasObject->childs[i]->GetComponent<C_UI_Image>();
				if (healthBarImage)
					healthMaxW = healthBarImage->GetRect().w;
				continue;
			}
			else if (!strcmp(healthBarCanvasObject->childs[i]->GetName(), bossIconStr.c_str()))
			{

				C_UI_Image* bossIcon = healthBarCanvasObject->childs[i]->GetComponent<C_UI_Image>();
				if (bossIcon)
					bossIcon->SetTextureCoordinates(-1372, -1821, 320, 320);
				continue;
			}
		}
	}

	//Audios
	damageAudio = new C_AudioSource(gameObject);
	deathAudio = new C_AudioSource(gameObject);
	walkAudio = new C_AudioSource(gameObject);
	bombAudio = new C_AudioSource(bombGameObject);
	if (damageAudio != nullptr)
		damageAudio->SetEvent("ig11_damaged");
	if (deathAudio != nullptr)
		deathAudio->SetEvent("ig11_death");
	if(bombAudio != nullptr)
		bombAudio->SetEvent("barrel_active");

	//Particles and SFX
	bombingParticles = bombGameObject->GetComponent<C_ParticleSystem>();
	(bombingParticles != nullptr) ? bombingParticles->StopSpawn() : LOG("[ERROR] IG12 Script: Could not find { BOMBING } Particle System!");

	hitParticles = gameObject->GetComponent<C_ParticleSystem>();
	(hitParticles != nullptr) ? hitParticles->StopSpawn() : LOG("[ERROR] IG12 Script: Could not find { HIT } Particle System!");
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

	if (damageAudio != nullptr)
		delete damageAudio;
	if (deathAudio != nullptr)
		delete deathAudio;
	if (walkAudio != nullptr)
		delete walkAudio;
	if (bombAudio != nullptr)
		delete bombAudio;
	
}

void IG12::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
		playerScript->TakeDamage(1.0f);
}

void IG12::TakeDamage(float damage)
{
	health -= damage / Defense();
	if (health < 0.0f)
		health = 0.0f;

	hitTimer.Start();
	if (hitParticles != nullptr)
		hitParticles->ResumeSpawn();

	if (damageAudio != nullptr)
		damageAudio->PlayFx(damageAudio->GetEventId());

	if (healthBarImage)
	{
		Rect2D rect = healthBarImage->GetRect();
		healthBarImage->SetW(healthMaxW * health / maxHealth);
	}
}

void IG12::EntityPause()
{
	firstStageTimer.Pause();
	secondStageTimer.Pause();
	spiralAttackTimer.Pause();
	lineAttackTimer.Pause();
	bombingAttackTimer.Pause();
	bombTimer.Pause();
	bombingAndSpiralAttackTimer.Pause();
}

void IG12::EntityResume()
{
	firstStageTimer.Resume();
	secondStageTimer.Resume();
	spiralAttackTimer.Resume();
	lineAttackTimer.Resume();
	bombingAttackTimer.Resume();
	bombTimer.Resume();
	bombingAndSpiralAttackTimer.Resume();
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
	if (dieAfterStun == 2)
	{
		dieAfterStun = 3;
		moveState = IG12State::DEAD_IN;
		deathTimer.Resume();
	}
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

	//Handle cutscene
	if (gameManager->dialogManager->GetDialogState() != DialogState::NO_DIALOG)
	{
		currentAnimation = &bombingAnimation;
		aimState = AimState::ON_GUARD;
		return;
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
		if (health <= (maxHealth * 0.5))
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}	
	else if (lineAttackTimer.IsActive() && lineAttackTimer.ReadSec() >= lineAttackDuration)
	{
		lineAttackTimer.Stop();
		lineAttackShots += 15;
		if (health <= (maxHealth * 0.5))
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}
	else if (bombingAttackTimer.IsActive() && bombingAttackTimer.ReadSec() >= bombingAttackDuration)
	{
		
		bombingAttackTimer.Stop();
		bombingAttackShots = 5;
		if (health <= (maxHealth * 0.5))
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}
	else if (bombingAndSpiralAttackTimer.IsActive() && bombingAndSpiralAttackTimer.ReadSec() >= bombingAttackDuration)
	{
		bombingAndSpiralAttackTimer.Stop();
		if (health <= (maxHealth * 0.5))
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}
	else if (!firstStageTimer.IsActive() && !secondStageTimer.IsActive())
	{
		if (health <= (maxHealth * 0.5))
			secondStageTimer.Start();
		else
			firstStageTimer.Start();
	}

	if (hitParticles != nullptr)
		if(!hitTimer.IsActive())
			hitParticles->StopSpawn();
	
	Player* tmp = nullptr;

	
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
		if (currentAnimation == &bombingAnimation && 2.0f < bombingAttackTimer.ReadSec())
			currentAnimation = &walkAnimation;
		else if (currentAnimation != &bombingAnimation)
			currentAnimation = &walkAnimation;

		if (distance < chaseDistance)
		{
			moveState = IG12State::CHASE;
			break;
		}
		Patrol();
		break;
	case IG12State::CHASE:
		if (currentAnimation == &bombingAnimation && 2.0f < bombingAttackTimer.ReadSec())
			currentAnimation = &runAnimation;
		else if(currentAnimation != &bombingAnimation)
			currentAnimation = &runAnimation;
		if (distance < attackDistance)
		{
			moveState = IG12State::IDLE;
			break;
		}
		Chase();
		break;
	case IG12State::FLEE:
		if (currentAnimation == &bombingAnimation && 2.0f < bombingAttackTimer.ReadSec())
			currentAnimation = &fleeAnimation;
		else if (currentAnimation != &bombingAnimation)
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
			if (health > (maxHealth * 0.5))
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
		currentAnimation = &bombingAnimation;
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		attackDistance = 200.0f;
		aimState = AimState::IDLE;

		projectileGameObject->SetIsActive(true);

		if (blasterWeapon)
		{
			blasterWeapon->fireRate = 0.0f;
			blasterWeapon->ammo = 0;
		}
		if (sniperWeapon)
		{
			sniperWeapon->fireRate = 0.0f;
			sniperWeapon->ammo = 0;
		}

		if (prepareBombing.IsActive() && prepareBombing.ReadSec() >= prepareBombingTime)
		{
			prepareBombing.Stop();
			bombingAttackTimer.Start();
			attackDistance = userAttackDistance;
			moveState = IG12State::BOMBING_ATTACK;
		}
		break;

	case IG12State::BOMBING_ATTACK:
		if (!BombingAttack())
		{
			moveState = IG12State::IDLE;
			attackDistance = userAttackDistance;

			projectileGameObject->SetIsActive(false);
			crosshair->SetIsActive(false);
			bombingParticles->StopSpawn();
			bombTimer.Stop();

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

			if (bombingParticles != nullptr)	//make sure particles stay in place
				bombingParticles->StopSpawn();

			bombExploding = false;				//make sure there is no infinite shake due to bomb explode not being reset to false

			//crosshair->transform->SetWorldPosition(float3(bombPosition.x, 500, bombPosition.y));	//hide crosshair by default
		}
		break;
	case IG12State::SPIRAL_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		currentAnimation = &specialAnimation;
		
		moveState = IG12State::SPIRAL_ATTACK;

	case IG12State::SPIRAL_ATTACK:
		if (!SpiralAttack())
		{
			moveState = IG12State::IDLE;

			if (health > (maxHealth * 0.5))
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
	case IG12State::BOMBING_AND_SPIRAL_ATTACK_IN:
		specialAttackStartAim = aimDirection;
		specialAttackRot = 0.0f;
		attackDistance = 200.0f;
		aimState = AimState::IDLE;
		currentAnimation = &bombingAnimation;

		if (blasterWeapon)
		{
			blasterWeapon->fireRate = 0.0f;
			blasterWeapon->ammo = 0;
		}
		if (sniperWeapon)
		{
			sniperWeapon->fireRate = 0.0f;
			sniperWeapon->ammo = 0;
		}

		if (prepareBombing.ReadSec() >= prepareBombingTime)
		{			
			attackDistance = userAttackDistance;
			prepareBombing.Stop();
			bombingAndSpiralAttackTimer.Start();
			moveState = IG12State::BOMBING_AND_SPIRAL_ATTACK;
		}
		break;
	case IG12State::BOMBING_AND_SPIRAL_ATTACK:
		currentAnimation = &specialAnimation;

		if (!BombingAndSpiralAttack())
		{
			moveState = IG12State::IDLE;

			projectileGameObject->SetIsActive(false);
			crosshair->SetIsActive(false);
			bombingParticles->StopSpawn();
			bombTimer.Stop();

			if (health > (maxHealth * 0.5))
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

			if (bombingParticles != nullptr)	//make sure particles stay in place
				bombingParticles->StopSpawn();

			bombExploding = false;				//make sure there is no infinite shake due to bomb explode not being reset to false

			//crosshair->transform->SetWorldPosition(float3(bombPosition.x, 500, bombPosition.y));	//hide crosshair by default
		}
		break;
	case IG12State::DEAD_IN:
		currentAnimation = &deathAnimation;
		deathTimer.Start();
		moveState = IG12State::DEAD;
		if (healthBarCanvas)
			App->uiSystem->RemoveActiveCanvas(healthBarCanvas);

		tmp = (Player*)player->GetScript("Player");
		if (tmp)
			tmp->GiveBeskar(4);

		gameManager->KilledIG11(1); //This will trigger timer to start to leave room

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
		aimState = AimState::SHOOT;

	case AimState::SHOOT:

		if (currentAnimation == &bombingAnimation && 2.0f < bombingAttackTimer.ReadSec())
			currentAnimation = &shootAnimation;
		else if (currentAnimation != &bombingAnimation)
			currentAnimation = &shootAnimation;

		if (moveState != IG12State::SPIRAL_ATTACK && moveState != IG12State::BOMBING_AND_SPIRAL_ATTACK)
			secondaryAimDirection = aimDirection;
		if (moveState == IG12State::LINE_ATTACK)
			lineAttackShots--;

		switch (blasterWeapon->Shoot(aimDirection))
		{
		case ShootState::NO_FULLAUTO:
			//currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::WAITING_FOR_NEXT:
			break;
		case ShootState::FIRED_PROJECTILE:
			//currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::RATE_FINISHED:
			//currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::NO_AMMO:
			aimState = AimState::RELOAD_IN;
			break;
		}
		switch (sniperWeapon->Shoot(secondaryAimDirection))
		{
		case ShootState::NO_FULLAUTO:
			//currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::WAITING_FOR_NEXT:
			break;
		case ShootState::FIRED_PROJECTILE:
			//currentAnimation = nullptr;
			aimState = AimState::ON_GUARD;
			break;
		case ShootState::RATE_FINISHED:
			//currentAnimation = nullptr;
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

	if (moveState == IG12State::SPIRAL_ATTACK || moveState == IG12State::BOMBING_AND_SPIRAL_ATTACK) currentAnimation = &specialAnimation;
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
	gameObject->GetComponent<C_RigidBody>()->StopInertia();

	specialAttackRot += spiralAttackSpeed * MC_Time::Game::GetDT();
	float angle = specialAttackStartAim.AimedAngle();
	angle += DegToRad(specialAttackRot);

	float x = cos(angle);
	float y = sin(angle);
	aimDirection = { x,y };

	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.01f;
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
	gameObject->GetComponent<C_RigidBody>()->StopInertia();
	
	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.1f;
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
	gameObject->GetComponent<C_RigidBody>()->StopInertia();

	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.3f;
		blasterWeapon->ammo = 20;
		blasterWeapon->projectilesPerShot = 1;
	}
	if (sniperWeapon)
	{
		sniperWeapon->fireRate = 0.3f;
		sniperWeapon->ammo = 20;
		sniperWeapon->projectilesPerShot = 1;
	}

	if (bombingParticles != nullptr)	//make sure particles stay in place
		bombingParticles->StopSpawn();
	bombExploding = false;				//make sure there is no infinite shake due to bomb explode not being reset to false
	bombCollider->SetIsActive(false);
	

	if (bombTimer.IsActive() && projectileGameObject->transform->GetWorldPosition().y < 2)
	{
		projectileGameObject->GetComponent<C_RigidBody>()->MakeStatic();
		
		/*if (bombingParticles != nullptr)
			bombingParticles->ResumeSpawn();

		bombExploding = true;*/
		
		bombTimer.Stop();

		bombCollider->SetIsActive(true);

		crosshair->SetIsActive(false);

		projectileGameObject->SetIsActive(false);

		if (bombAudio != nullptr)
			bombAudio->PlayFx(bombAudio->GetEventId());
			

	}
	else if (bombTimer.IsActive() && projectileGameObject->transform->GetWorldPosition().y < 10)
	{
		if (bombingParticles != nullptr)
			bombingParticles->ResumeSpawn();
		
		bombExploding = true;
	}
	else if (!bombTimer.IsActive())
	{
		playerPosition.x = player->transform->GetWorldPosition().x;
		playerPosition.y = player->transform->GetWorldPosition().z;
		bombTimer.Start();
		crosshair->SetIsActive(true);
		float3 bombPos = CalculateNextBomb(playerPosition.x, playerPosition.y);
		bombGameObject->transform->SetWorldPosition(bombPos);
		projectileGameObject->SetIsActive(true);
		projectileGameObject->transform->SetWorldPosition(float3(bombPos.x, 100, bombPos.z));
		projectileGameObject->GetComponent<C_RigidBody>()->MakeDynamic();
		projectileGameObject->GetComponent<C_RigidBody>()->AddForce(float3(0, -0.07, 0));
	}

	if (!bombingAttackTimer.IsActive())
		return false;

	return true;
}

float3 IG12::CalculateNextBomb(float x, float y)
{
	float bombX = Lerp(x - (bombingAttackBigAreaSide * 0.5), x + (bombingAttackBigAreaSide * 0.5), randomGenerator.Float());
	float bombY = Lerp(y - (bombingAttackBigAreaSide * 0.5), y + (bombingAttackBigAreaSide * 0.5), randomGenerator.Float());
	return float3(bombX,0, bombY);
}

bool IG12::BombingAndSpiralAttack()
{
	gameObject->GetComponent<C_RigidBody>()->StopInertia();
	
	specialAttackRot += spiralAttackSpeed * MC_Time::Game::GetDT();
	float angle = specialAttackStartAim.AimedAngle();
	angle += DegToRad(specialAttackRot);

	float x = cos(angle);
	float y = sin(angle);
	aimDirection = { x,y };

	if (blasterWeapon)
	{
		blasterWeapon->fireRate = 0.01f;
		blasterWeapon->ammo = 20;
		blasterWeapon->projectilesPerShot = 2;
		blasterWeapon->shotSpreadArea = 4;
	}
	if (sniperWeapon)
	{
		sniperWeapon->fireRate = 0.01f;
		sniperWeapon->ammo = 20;
		sniperWeapon->projectilesPerShot = 2;
		sniperWeapon->shotSpreadArea = 4;
		secondaryAimDirection = -aimDirection;
	}


	if (bombingParticles != nullptr)	//make sure particles stay in place
		bombingParticles->StopSpawn();
	bombExploding = false;				//make sure there is no infinite shake due to bomb explode not being reset to false
	bombCollider->SetIsActive(false);


	if (bombTimer.IsActive() && projectileGameObject->transform->GetWorldPosition().y < 2)
	{
		projectileGameObject->GetComponent<C_RigidBody>()->MakeStatic();


		bombTimer.Stop();

		bombCollider->SetIsActive(true);

		crosshair->SetIsActive(false);

		projectileGameObject->SetIsActive(false);

		if (bombAudio != nullptr)
			bombAudio->PlayFx(bombAudio->GetEventId());
			

	}
	else if (bombTimer.IsActive() && projectileGameObject->transform->GetWorldPosition().y < 10)
	{
		if (bombingParticles != nullptr)
			bombingParticles->ResumeSpawn();

		bombExploding = true;
	}
	else if (!bombTimer.IsActive())
	{
		playerPosition.x = player->transform->GetWorldPosition().x;
		playerPosition.y = player->transform->GetWorldPosition().z;
		bombTimer.Start();
		crosshair->SetIsActive(true);
		float3 bombPos = CalculateNextBomb(playerPosition.x, playerPosition.y);
		bombGameObject->transform->SetWorldPosition(bombPos);
		projectileGameObject->SetIsActive(true);
		projectileGameObject->transform->SetWorldPosition(float3(bombPos.x, 100, bombPos.z));
		projectileGameObject->GetComponent<C_RigidBody>()->MakeDynamic();
		projectileGameObject->GetComponent<C_RigidBody>()->AddForce(float3(0, -0.07, 0));
	}

	if (!bombingAndSpiralAttackTimer.IsActive() || specialAttackRot >= 360.0f * spiralAttackSpins)
	{
		projectileGameObject->SetIsActive(false);
		crosshair->SetIsActive(false);
		bombingParticles->StopSpawn();
		bombTimer.Stop();
		return false;
	}


	return true;
}

void IG12::pickFirstStageAttack()
{
	//bombing attack, straight line attack
	float randAttack = Lerp(0, 2, randomGenerator.Float());

	if (randAttack <= 1)
	{
		prepareBombing.Start();
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
		prepareBombing.Start();
		moveState = IG12State::BOMBING_AND_SPIRAL_ATTACK_IN;
	}
	else
	{
		spiralAttackTimer.Start();
		moveState = IG12State::SPIRAL_ATTACK_IN;
	}
}



