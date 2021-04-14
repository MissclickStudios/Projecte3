#pragma once
#include "ScriptMacros.h"

#include "Entity.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float2.h"

class C_RigidBody;

enum class PlayerMoveState
{
	IDLE,
	RUN,
	DASH_IN,
	DASH,
	DEAD_IN,
	DEAD
};

enum class PlayerAimState
{
	IDLE,
	SHOOT,
	RELOAD,
	CHANGE
};

class SCRIPTS_API Player : public Entity ALLOWED_INHERITANCE
{
public:

	Player();
	virtual ~Player();

	void Start();
	void Update();
	void CleanUp();

	// Interactions
	void TakeDamage(float damage);

	// Effects
	void Frozen();

	// Dash
	float dashSpeed = 0.0f;
	float dashDuration = 0.0f;
	float dashCooldown = 0.0f;

	// Animations
	std::string runAnimation = "Run";
	std::string dashAnimation = "Dash";
	std::string shootAnimation = "Shoot";

private:

	PlayerMoveState moveState = PlayerMoveState::IDLE;
	PlayerAimState aimState = PlayerAimState::IDLE;

	// Inputs & State Selection
	void GatherMoveInputs();
	void GatherAimInputs();

	float2 moveInput = float2::zero;
	float2 aimInput = float2::zero;

	// Movement
	void Movement();

	float2 moveDirection = float2::zero;
	float2 aimDirection = float2::zero;

	C_RigidBody* rigidBody = nullptr;

	// Dash
	void Dash();
	Timer dashTimer;
	Timer dashCooldownTimer;

	// Animations
	std::string* currentAnimation = nullptr;
};

SCRIPTS_FUNCTION Player* CreatePlayer()
{
	Player* script = new Player();
	return script;
}