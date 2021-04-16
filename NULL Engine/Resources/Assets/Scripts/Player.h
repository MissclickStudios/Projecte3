#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "MathGeoLib/include/Math/float2.h"

enum class PlayerState
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

	// Dash
	float dashSpeed = 0.0f;
	float DashSpeed() { return dashSpeed * speedModifier; }
	float dashDuration = 0.0f;
	float DashDuration() { return dashDuration / speedModifier; }
	float dashCooldown = 0.0f;
	float DashCooldown() { return dashCooldown / cooldownModifier; }

	// Animations
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo dashAnimation = { "Dash" };
	AnimationInfo shootAnimation = { "Shoot" };
	AnimationInfo reloadAnimation = { "Reload" };
	AnimationInfo changeAnimation = { "Change" };
	AnimationInfo onGuardAnimation = { "OnGuard" };

private:
	Timer POOPOOTIMER; // for testing porpouses, mom found the poop sock :skull:
	PlayerState moveState = PlayerState::IDLE;
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