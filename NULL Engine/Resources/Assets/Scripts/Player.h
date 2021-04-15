#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "MathGeoLib/include/Math/float2.h"

enum class PlayerMoveState
{
	IDLE,
	RUN,
	DASH_IN,
	DASH,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API Player : public Entity ALLOWED_INHERITANCE
{
public:

	Player();
	virtual ~Player();

	void SetUp();
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
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo dashAnimation = { "Dash" };
	AnimationInfo shootAnimation = { "Shoot" };

private:
	Timer POOPOOTIMER; // for testing porpouses, mom found the poop sock :skull:
	PlayerMoveState moveState = PlayerMoveState::IDLE;
	AimState aimState = AimState::IDLE;

	// Logic
	void ManageMovement();
	void ManageAim();

	// Inputs & State Selection
	void GatherMoveInputs();
	void GatherAimInputs();

	float2 moveInput = float2::zero;
	float2 aimInput = float2::zero;

	// Movement
	void Movement();

	float2 moveDirection = float2::zero;
	float2 aimDirection = float2::zero;

	// Dash
	void Dash();
	Timer dashTimer;
	Timer dashCooldownTimer;
};

SCRIPTS_FUNCTION Player* CreatePlayer();