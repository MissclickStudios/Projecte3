#include "Player.h"

#include "Application.h"
#include "M_Input.h"

#include "GameObject.h"
#include "C_RigidBody.h"

#include "C_Animator.h"

#include "Log.h"

#define MAX_INPUT 32767

Player::Player() : Entity()
{
	POOPOOTIMER.Stop(); // hehe xd
	dashTimer.Stop();
	dashCooldownTimer.Stop();
}

Player::~Player()
{
}

void Player::Start()
{
	rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (rigidBody && rigidBody->IsStatic())
		rigidBody = nullptr;

	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	speed = 20.0f;

	dashSpeed = 80.0f;
	dashDuration = 0.2f;
	dashCooldown = 0.5f;
}

void Player::Update()
{
	ManageMovement();
	ManageAim();

	if (animator && currentAnimation)
	{
		AnimatorClip* clip = animator->GetCurrentClip();
		if (clip)
		{
			std::string clipName = clip->GetName();
			if (clipName != *currentAnimation)	// If the animtion changed play the wanted clip
				animator->PlayClip(*currentAnimation, 0.2f);
		}
		else
			animator->PlayClip(*currentAnimation, 0.2f); // If there is no clip playing play the current animation
	}
}

void Player::CleanUp()
{
}

void Player::TakeDamage(float damage)
{
}

void Player::Frozen()
{
}

void Player::ManageMovement()
{
	if (moveState != PlayerMoveState::DEAD)
	{
		if (moveState != PlayerMoveState::DASH)
			GatherMoveInputs();

		if (health <= 0.0f)
			moveState == PlayerMoveState::DEAD_IN;
	}

	if (rigidBody)
		switch (moveState)
		{
		case PlayerMoveState::IDLE:
			rigidBody->SetLinearVelocity(float3::zero);
			currentAnimation = &idleAnimation;
			break;
		case PlayerMoveState::RUN:
			Movement();
			currentAnimation = &runAnimation;
			break;
		case PlayerMoveState::DASH_IN: // The first frame after dashing
			Dash();
			dashTimer.Start();
			moveState = PlayerMoveState::DASH;
			currentAnimation = &dashAnimation;
			break;
		case PlayerMoveState::DASH:
			if (dashTimer.ReadSec() >= dashDuration) // When the dash duration ends start the cooldown and reset the move state
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();

				moveState = PlayerMoveState::IDLE;
			}
			break;
		case PlayerMoveState::DEAD_IN: // The first frame after dying
			rigidBody->SetIsActive(false); // Disable the rigidbody to avoid more interactions with other entities
			moveState = PlayerMoveState::DEAD;
			deathTimer.Start();
			currentAnimation = &deathAnimation;
			break;
		case PlayerMoveState::DEAD:
			if (deathTimer.ReadSec() >= deathDuration)
			{
				// DYING THINGS
			}
			break;
		}
}

void Player::ManageAim()
{
	if (aimState == PlayerAimState::IDLE || aimState == PlayerAimState::ON_GUARD)
	{
		GatherAimInputs();
	}

	switch (aimState)
	{
	case PlayerAimState::IDLE:
		break;
	case PlayerAimState::ON_GUARD:
		aimState = PlayerAimState::IDLE;
		break;
	case PlayerAimState::SHOOT_IN:
		currentAnimation = &shootAnimation;
		POOPOOTIMER.Start();
		aimState = PlayerAimState::SHOOT;
		break;
	case PlayerAimState::SHOOT:
		currentAnimation = &shootAnimation; // temporary till torso gets an independent animator
		if (POOPOOTIMER.ReadSec() >= 0.6f)
		{
			aimState = PlayerAimState::ON_GUARD;
			POOPOOTIMER.Stop();
			currentAnimation = nullptr;
		}
		break;
	case PlayerAimState::RELOAD_IN:
		aimState = PlayerAimState::RELOAD;
		break;
	case PlayerAimState::RELOAD:
		aimState = PlayerAimState::ON_GUARD;
		break;
	case PlayerAimState::CHANGE_IN:
		aimState = PlayerAimState::CHANGE;
		break;
	case PlayerAimState::CHANGE:
		aimState = PlayerAimState::ON_GUARD;
		break;
	}
}

void Player::GatherMoveInputs()
{
	// Controller movement
	int movX = App->input->GetGameControllerAxisValue(0);
	int movY = App->input->GetGameControllerAxisValue(1);
	// Keyboard movement
	if (movX == 0 && movY == 0)	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)
			movY = -MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)
			movY = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)
			movX = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)
			movX = -MAX_INPUT;
	}
	moveInput = { (float)movX, (float)movY };

	if (!dashCooldownTimer.IsActive())
	{
		if ((App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_DOWN
			|| App->input->GetGameControllerTrigger(0) == ButtonState::BUTTON_DOWN))
		{
			if (!dashTimer.IsActive())
				moveState = PlayerMoveState::DASH_IN;

			return;
		}
	}
	else if (dashCooldownTimer.ReadSec() >= dashCooldown)
		dashCooldownTimer.Stop();

	if (!moveInput.IsZero()) // Set the player in a running state if it's not dashing
	{
		moveState = PlayerMoveState::RUN;
		return;
	}

	moveState = PlayerMoveState::IDLE;
}

void Player::GatherAimInputs()
{
	// Controller aim
	int aimX = App->input->GetGameControllerAxisValue(2);
	int aimY = App->input->GetGameControllerAxisValue(3);
	// Keyboard aim
	if (aimX == 0 && aimY == 0)	// If there was no controller input
	{
		if (App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_REPEAT)
			aimY = -MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_REPEAT)
			aimY = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_REPEAT)
			aimX = MAX_INPUT;
		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_REPEAT)
			aimX = -MAX_INPUT;
	}
	aimInput = { (float)aimX, (float)aimY };

	if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN)
	{
		aimState = PlayerAimState::CHANGE_IN;
		return;
	}

	if ((App->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(2) == ButtonState::BUTTON_DOWN))
	{
		aimState = PlayerAimState::RELOAD_IN;
		return;
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KeyState::KEY_REPEAT || App->input->GetGameControllerTrigger(1) == ButtonState::BUTTON_REPEAT)
	{
		aimState = PlayerAimState::SHOOT_IN;
		return;
	}

	aimState = PlayerAimState::IDLE;
}

void Player::Movement()
{
	float3 direction = { moveInput.x, 0, moveInput.y };
	direction.Normalize();
	moveDirection = { moveInput.x, moveInput.y }; // Save the value

	direction *= Speed(); // Apply the processed speed value to the unitari direction vector
	rigidBody->SetLinearVelocity(direction);
}

void Player::Dash()
{
	float3 direction = { moveDirection.x, 0, moveDirection.y };
	direction.Normalize();

	rigidBody->SetLinearVelocity(direction * dashSpeed);
}
