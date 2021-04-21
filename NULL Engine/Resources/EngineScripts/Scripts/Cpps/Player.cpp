#include "Player.h"

#include "Application.h"
#include "M_Input.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

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

	rigidBody->TransformMovesRigidBody(false);

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
	blasterGameObject = App->resourceManager->LoadPrefab(blaster.uid, App->scene->GetSceneRoot());
	equipedGunGameObject = App->resourceManager->LoadPrefab(equipedGun.uid, App->scene->GetSceneRoot());

	if (blasterGameObject)
		blasterWeapon = (Weapon*)GetObjectScript(blasterGameObject, ObjectType::WEAPON);
	if (blasterWeapon)
		blasterWeapon->SetOwnership(type, hand);
	if (equipedGunGameObject)
		equipedGunWeapon = (Weapon*)GetObjectScript(equipedGunGameObject, ObjectType::WEAPON);
	if (blasterWeapon)
		equipedGunWeapon->SetOwnership(type, hand);

	currentWeapon = blasterWeapon;
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

void Player::TakeDamage(float damage)
{
	if (invencibilityTimer.ReadSec() >= invencibilityDuration)
		invencibilityTimer.Stop();
	if (!invencibilityTimer.IsActive())
	{
		health -= damage / Defense();
		if (health < 0.0f)
			health = 0.0f;
		invencibilityTimer.Start();
	}
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
			currentAnimation = &dashAnimation;
			Dash();
			dashTimer.Start();
			if (rigidBody)
				rigidBody->ChangeFilter(" player dashing");
			moveState = PlayerState::DASH;

		case PlayerState::DASH:
			if (dashTimer.ReadSec() >= DashDuration()) // When the dash duration ends start the cooldown and reset the move state
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();
				if (rigidBody)
					rigidBody->ChangeFilter(" player");
				moveState = PlayerState::IDLE;
			}
			break;
		case PlayerState::DEAD_IN:
			currentAnimation = &deathAnimation;
			if (rigidBody)
				rigidBody->SetIsActive(false); // Disable the rigidbody to avoid more interactions with other entities
			deathTimer.Start();
			moveState = PlayerState::DEAD;

		case PlayerState::DEAD:
			if (deathTimer.ReadSec() >= deathDuration)
			{
				// DIE ALREADY !
				GameObject* gameManagerObject = App->scene->GetGameObjectByName("Game Manager");
				GameManager* gameManagerScript = (GameManager*)gameManagerObject->GetScript("GameManager");
				gameManagerScript->GoNextRoom();
			}
			break;
		}
}

void Player::ManageAim()
{
	if (aimState == AimState::IDLE || aimState == AimState::ON_GUARD)
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
		changeTimer.Start();
		aimState = AimState::CHANGE;

	case AimState::CHANGE:
		if (changeTimer.ReadSec() >= ChangeTime())
		{
			if (blasterWeapon == currentWeapon)
				currentWeapon = equipedGunWeapon;
			else
				currentWeapon = blasterWeapon;
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
	if (aimInput.IsZero())
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
