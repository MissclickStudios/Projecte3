#include "JSONParser.h"

#include "Player.h"

#include "Application.h"
#include "M_Input.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_Material.h"
#include "C_ParticleSystem.h"
#include "Emitter.h"
#include "C_AudioSource.h"

#include "C_Animator.h"

#include "GameManager.h"

#include "Log.h"

#define MAX_INPUT 32767

Player* CreatePlayer()
{
	Player* script = new Player();

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

	// Player ---

	// Currency
	INSPECTOR_DRAGABLE_INT(script->currency);
	INSPECTOR_DRAGABLE_INT(script->hubCurrency);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->dashCooldown);

	// Invencibility frames
	INSPECTOR_DRAGABLE_FLOAT(script->invencibilityDuration);

	// Weapons
	INSPECTOR_DRAGABLE_FLOAT(script->changeTime);

	INSPECTOR_PREFAB(script->blaster);
	INSPECTOR_PREFAB(script->equipedGun);

	INSPECTOR_STRING(script->gameManager);

	//// Animations ---
	//// Movement
	//INSPECTOR_STRING(script->idleAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->idleAnimation.blendTime);
	//INSPECTOR_STRING(script->runAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->runAnimation.blendTime);
	//INSPECTOR_STRING(script->dashAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->dashAnimation.blendTime);
	//INSPECTOR_STRING(script->deathAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->deathAnimation.blendTime);
	//
	//// Aim
	//INSPECTOR_STRING(script->shootAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->shootAnimation.blendTime);
	//INSPECTOR_STRING(script->reloadAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->reloadAnimation.blendTime);
	//INSPECTOR_STRING(script->changeAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->changeAnimation.blendTime);
	//INSPECTOR_STRING(script->onGuardAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->onGuardAnimation.blendTime);

	return script;
}

Player::Player() : Entity()
{
	type = EntityType::PLAYER;
}

Player::~Player()
{
}

void Player::SetUp()
{
	dashTimer.Stop();
	dashCooldownTimer.Stop();
	invencibilityTimer.Stop();
	changeTimer.Stop();

	if (rigidBody)
		rigidBody->TransformMovesRigidBody(false);

	if (particles)
		for (uint i = 0; i < particles->emitterInstances.size(); ++i)
			if (particles->emitterInstances[i]->emitter->name == "Dash")
				dashParticles = particles->emitterInstances[i];

	currentWeapon = blasterWeapon;

	for (uint i = 0; i < gameObject->components.size(); ++i)
	{
		if (gameObject->components[i]->GetType() == ComponentType::AUDIOSOURCE)
		{
			C_AudioSource* source = (C_AudioSource*)gameObject->components[i];
			std::string name = source->GetEventName();

			if (name == "mando_walking")
				walkAudio = source;
			else if (name == "mando_dash")
				dashAudio = source;
			else if (name == "weapon_change")
				changeWeaponAudio = source;
			else if (name == "mando_damaged")
				damageAudio = source;
			else if (name == "mando_death")
				deathAudio = source;
		}
	}
}

void Player::Update()
{
	ManageMovement();
	if (moveState != PlayerState::DEAD)
		ManageAim();
}

void Player::CleanUp()
{
	if (blasterGameObject)
		blasterGameObject->toDelete = true;
	blasterGameObject = nullptr;
	blasterWeapon = nullptr;

	if (equipedGunGameObject)
		equipedGunGameObject->toDelete = true;
	equipedGunGameObject = nullptr;
	equipedGunWeapon = nullptr;

	currentWeapon = nullptr;
}

void Player::SaveState(ParsonNode& playerNode)
{
	playerNode.SetInteger("Currency", currency);
	playerNode.SetInteger("Hub Currency", hubCurrency);

	playerNode.SetNumber("Health", health);

	playerNode.SetBool("God Mode", godMode);

	ParsonArray effectsArray = playerNode.SetArray("Effects");
	for (uint i = 0; i < effects.size(); ++i)
	{
		ParsonNode node = effectsArray.SetNode("Effect");
		node.SetInteger("Type", (int)effects[i]->Type());
		node.SetNumber("Duration", (double)effects[i]->RemainingDuration());
		node.SetBool("Permanent", effects[i]->Permanent());
	}

	ParsonArray blasterPerks = playerNode.SetArray("Blaster Perks");
	if (blasterWeapon)
	{
		playerNode.SetInteger("Blaster Ammo", blasterWeapon->ammo);
		for (uint i = 0; i < blasterWeapon->perks.size(); ++i)
			blasterPerks.SetNumber((double)blasterWeapon->perks[i]);
	}

	playerNode.SetInteger("Equiped Gun", equipedGun.uid);
	ParsonArray equipedGunPerks = playerNode.SetArray("Equiped Gun Perks");
	if (equipedGunWeapon)
	{
		playerNode.SetInteger("Equiped Gun Ammo", equipedGunWeapon->ammo);
		for (uint i = 0; i < equipedGunWeapon->perks.size(); ++i)
			equipedGunPerks.SetNumber((double)equipedGunWeapon->perks[i]);
	}
}

void Player::LoadState(ParsonNode& playerNode)
{
	currency = playerNode.GetInteger("Currency");
	hubCurrency = playerNode.GetInteger("Hub Currency");

	health = playerNode.GetNumber("Health");

	godMode = playerNode.GetBool("God Mode");
	SetGodMode(godMode);

	ParsonArray effectsArray = playerNode.GetArray("Effects");
	for (uint i = 0; i < effectsArray.size; ++i)
	{
		ParsonNode node = effectsArray.GetNode(i);
		EffectType type = (EffectType)node.GetInteger("Type");
		float duration = node.GetNumber("Duration");
		bool permanent = node.GetBool("Permanent");

		AddEffect(type, duration, permanent);
	}

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

	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());
	if (blasterGameObject)
	{
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);

		if (blasterWeapon)
		{
			blasterWeapon->SetOwnership(type, hand);
			blasterWeapon->ammo = playerNode.GetInteger("Blaster Ammo");

			ParsonArray blasterPerks = playerNode.GetArray("Blaster Perks");
			for (uint i = 0; i < blasterPerks.size; ++i)
				blasterWeapon->AddPerk((Perk)(int)blasterPerks.GetNumber(i));
		}
	}

	// TODO: Load correct secondary gun
	//equipedGunGameObject = App->resourceManager->LoadPrefab(playerNode.GetInteger("Equiped Gun"), App->scene->GetSceneRoot());
	equipedGunGameObject = App->resourceManager->LoadPrefab(equipedGun.uid, App->scene->GetSceneRoot());
	if (equipedGunGameObject)
	{
		equipedGunWeapon = (Weapon*)GetObjectScript(equipedGunGameObject, ObjectType::WEAPON);

		if (equipedGunWeapon)
		{
			equipedGunWeapon->SetOwnership(type, hand);
			equipedGunWeapon->ammo = playerNode.GetInteger("Equiped Gun Ammo");

			ParsonArray equipedGunPerks = playerNode.GetArray("Equiped Gun Perks");
			for (uint i = 0; i < equipedGunPerks.size; ++i)
				equipedGunWeapon->AddPerk((Perk)(int)equipedGunPerks.GetNumber(i));

			if (equipedGunWeapon->weaponModel)
				equipedGunWeapon->weaponModel->SetIsActive(false);
		}
	}
}

void Player::Reset()
{
	currency = 0;

	health = maxHealth;

	while (effects.size())
	{
		delete* effects.begin();
		effects.erase(effects.begin());
	}

	if (blasterWeapon)
	{
		blasterWeapon->ammo = blasterWeapon->maxAmmo;
		blasterWeapon->perks.clear();
	}
	if (equipedGunWeapon)
	{
		equipedGunWeapon->ammo = equipedGunWeapon->maxAmmo;
		equipedGunWeapon->perks.clear();
	}
}

void Player::TakeDamage(float damage)
{
	if (invencibilityTimer.ReadSec() >= invencibilityDuration)
		invencibilityTimer.Stop();
	if (!invencibilityTimer.IsActive())
	{
		float damageDealt = 0.0f;
		if(Defense())
		 damageDealt = damage / Defense();
		health -= damageDealt;

		if (health < 0.0f)
			health = 0.0f;
		invencibilityTimer.Start();

		hitTimer.Start();
		if (hitParticles)
			hitParticles->stopSpawn = false;
		if (material)
		{
			material->SetAlternateColour(Color(1, 0, 0, 1));
			material->SetTakeDamage(true);
		}

		if (damageAudio)
			damageAudio->PlayFx(damageAudio->GetEventId());
	}
}

void Player::SetGodMode(bool enable)
{
	godMode = enable;
	if (godMode)
	{
		
		defense = 0.0f;
		
	}
	else
	{
		defense = 1.0f;
		
	}
}

bool Player::GetGodMode() const
{
	return godMode;
}

void Player::ManageMovement()
{
	if (moveState != PlayerState::DEAD)
	{
		if (health <= 0.0f)
			moveState = PlayerState::DEAD_IN;
		else
			if (moveState != PlayerState::DASH)
				GatherMoveInputs();
	}

	if (rigidBody)
		switch (moveState)
		{
		case PlayerState::IDLE:
			currentAnimation = &idleAnimation;
			if (rigidBody)
				rigidBody->SetLinearVelocity(float3::zero);
			break;
		case PlayerState::RUN:
			currentAnimation = &runAnimation;
			Movement();
			break;
		case PlayerState::DASH_IN:
			if (dashAudio)
				dashAudio->PlayFx(dashAudio->GetEventId());

			currentAnimation = &dashAnimation;
			dashTimer.Start();
			if (rigidBody)
				rigidBody->ChangeFilter(" player dashing");
			if (dashParticles)
				dashParticles->stopSpawn = false;
			moveState = PlayerState::DASH;

		case PlayerState::DASH:
			Dash();
			if (dashTimer.ReadSec() >= DashDuration()) // When the dash duration ends start the cooldown and reset the move state
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();
				if (rigidBody)
					rigidBody->ChangeFilter(" player");
				if (dashParticles)
					dashParticles->stopSpawn = true;
				moveState = PlayerState::IDLE;
			}
			break;
		case PlayerState::DEAD_IN:
			if (deathAudio)
				deathAudio->PlayFx(deathAudio->GetEventId());

			currentAnimation = &deathAnimation;
			if (rigidBody)
				rigidBody->SetIsActive(false); // Disable the rigidbody to avoid more interactions with other entities
			deathTimer.Start();
			moveState = PlayerState::DEAD;

		case PlayerState::DEAD:
			if (deathTimer.ReadSec() >= deathDuration)
				moveState = PlayerState::DEAD_OUT;
			break;
		}
}

void Player::ManageAim()
{
	if (moveState != PlayerState::DASH)
		GatherAimInputs();
	Aim();

	switch (aimState)
	{
	case AimState::IDLE:
		break;
	case AimState::ON_GUARD:
		aimState = AimState::IDLE;
		break;
	case AimState::SHOOT_IN:
		currentAnimation = &shootAnimation;
		aimState = AimState::SHOOT;

	case AimState::SHOOT:
		currentAnimation = &shootAnimation; // temporary till torso gets an independent animator
		if (currentWeapon)
			switch (currentWeapon->Shoot(aimDirection))
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
		if (currentWeapon && currentWeapon->Reload())
			aimState = AimState::ON_GUARD;
		break;
	case AimState::CHANGE_IN:
		if (changeWeaponAudio)
			changeWeaponAudio->PlayFx(changeWeaponAudio->GetEventId());

		changeTimer.Start();
		aimState = AimState::CHANGE;

	case AimState::CHANGE:
		if (changeTimer.ReadSec() >= ChangeTime())
		{
			if (blasterWeapon == currentWeapon)
			{
				currentWeapon = equipedGunWeapon;
				if (blasterWeapon->weaponModel)
					blasterWeapon->weaponModel->SetIsActive(false);
				if (equipedGunWeapon->weaponModel)
					equipedGunWeapon->weaponModel->SetIsActive(true);
			}
			else
			{
				currentWeapon = blasterWeapon;
				if (blasterWeapon->weaponModel)
					blasterWeapon->weaponModel->SetIsActive(true);
				if (equipedGunWeapon->weaponModel)
					equipedGunWeapon->weaponModel->SetIsActive(false);
			}
			aimState = AimState::ON_GUARD;
		}
		break;
	}
}

void Player::GatherMoveInputs()
{
	// Controller movement
	moveInput.x = (float)App->input->GetGameControllerAxisValue(0);
	moveInput.y = (float)App->input->GetGameControllerAxisValue(1);
	// Keyboard movement
	if (moveInput.IsZero())	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
			moveInput.y = -MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
			moveInput.y = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
			moveInput.x = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
			moveInput.x = -MAX_INPUT;
	}

	if (!dashCooldownTimer.IsActive())
	{
		if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN
			|| App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN))
		{
			if (!dashTimer.IsActive())
				moveState = PlayerState::DASH_IN;

			return;
		}
	}
	else if (dashCooldownTimer.ReadSec() >= DashCooldown())
		dashCooldownTimer.Stop();

	if (!moveInput.IsZero())
	{
		moveState = PlayerState::RUN;
		return;
	}

	moveState = PlayerState::IDLE;
}

void Player::GatherAimInputs()
{
	// Controller aim
	aimInput.x = (float)App->input->GetGameControllerAxisValue(2);
	aimInput.y = (float)App->input->GetGameControllerAxisValue(3);
	// Keyboard aim
	if (aimInput.IsZero())	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)
			aimInput.y = -MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)
			aimInput.y = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)
			aimInput.x = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)
			aimInput.x = -MAX_INPUT;
	}

	if (!(aimState == AimState::IDLE || aimState == AimState::ON_GUARD)) // If the player is not on this states, ignore action inputs (shoot, reload, etc.)
		return;

	if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN)
	{
		aimState = AimState::CHANGE_IN;
		return;
	}

	if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN))
	{
		aimState = AimState::RELOAD_IN;
		return;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
	{
		aimState = AimState::SHOOT_IN;
		return;
	}

	aimState = AimState::IDLE;
}

void Player::Movement()
{
	float3 direction = { moveInput.x, 0, moveInput.y };
	direction.Normalize();
	moveDirection = { moveInput.x, moveInput.y }; // Save the value

	direction *= Speed(); // Apply the processed speed value to the unitari direction vector
	if (rigidBody)
		rigidBody->SetLinearVelocity(direction);
}

void Player::Aim()
{
	if (aimInput.IsZero() || moveState == PlayerState::DASH) // We force the player to look in the direction he is dashing if he is
		aimDirection = moveDirection; // TEMPORARY
	else
		aimDirection = aimInput;

	float rad = aimDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad + DegToRad(90), 0));
}

void Player::Dash()
{
	float3 direction = { moveDirection.x, 0, moveDirection.y };
	direction.Normalize();

	if (rigidBody)
		rigidBody->SetLinearVelocity(direction * DashSpeed());
}
