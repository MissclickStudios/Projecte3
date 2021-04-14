#include "Player.h"

#include "Application.h"
#include "M_Input.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Animator.h"

#define MAX_INPUT 32767

Player::Player() : Entity()
{
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

	speed = 20.0f;

	dashSpeed = 80.0f;
	dashDuration = 0.2f;
	dashCooldown = 0.5f;
}

void Player::Update()
{
	if (moveState != PlayerMoveState::DEAD)
	{
		if (moveState != PlayerMoveState::DASH)
			GatherMoveInputs();
		GatherAimInputs();

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
		case PlayerMoveState::DASH_IN:
			Dash();
			dashTimer.Start();
			moveState = PlayerMoveState::DASH;
			currentAnimation = &dashAnimation;
			break;
		case PlayerMoveState::DASH:
			if (dashTimer.ReadSec() >= dashDuration)
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();

				moveState = PlayerMoveState::IDLE;
			}
			break;
		case PlayerMoveState::DEAD_IN:
			rigidBody->SetIsActive(false);
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

	if (animator)
	{
		AnimatorClip* clip = animator->GetCurrentClip();
		std::string clipName = "[NONE]"; 
		if (clip)
			clipName = clip->GetName();
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
