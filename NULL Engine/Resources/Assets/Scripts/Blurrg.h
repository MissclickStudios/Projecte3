#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"

#include "MathGeoLib/include/Math/float2.h"

class GameObject;

enum class BlurrgState
{
	WANDER,
	CHASE,
	CHARGE_IN,
	CHARGE,
	DASH_IN,
	DASH,
	REST_IN,
	REST,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API Blurrg : public Entity ALLOWED_INHERITANCE
{
public:

	Blurrg();
	virtual ~Blurrg();

	void SetUp();
	void Update();
	void CleanUp();

	// Interactions
	void TakeDamage(float damage);

	// Effects
	void Frozen();

	// Wander
	float wanderRadius = 0.0f;

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	GameObject* player;

	// Charge
	float chargeDistance = 0.0f;
	float chargeDuration = 0.0f;

	// Dash
	float dashSpeed = 0.0f;
	float dashDuration = 0.0f;
	float dashCooldown = 0.0f;

	// Rest
	float restDuration = 0.0f;

	// Animations
	AnimationInfo walkAnimation = { "Walk" };
	AnimationInfo chargeAnimation = { "Charge" };
	AnimationInfo dashAnimation = { "Dash" };
	AnimationInfo restAnimation = { "Rest" };

private:

	void LookAtPlayer();

	BlurrgState state = BlurrgState::WANDER;

	// Movement
	void Wander();
	void Chase();

	float distance = 0.0f;
	float2 moveDirection = float2::zero;

	// Charge
	Timer chargeTimer;

	// Dash
	void Dash();

	Timer dashTimer;
	Timer dashCooldownTimer;

	// Rest
	Timer restTimer;
};

SCRIPTS_FUNCTION Blurrg* CreateBlurrg();