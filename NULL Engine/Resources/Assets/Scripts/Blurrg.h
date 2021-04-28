#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;

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

	void SetUp() override;
	void Update() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	// Wander
	float wanderRadius = 0.0f;

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	float ChaseSpeed() { return Speed() * chaseSpeedModifier; }
	std::string playerName = "Mando testbuild";

	// Charge
	float chargeDistance = 0.0f;
	float chargeDuration = 0.0f;
	float ChargeDuration() { return chargeDuration / attackSpeedModifier; }

	// Dash
	float dashDamageModifier = DEFAULT_MODIFIER;
	float DashDamage() { return Damage() * dashDamageModifier; }
	float dashSpeed = 0.0f;
	float DashSpeed() { return dashSpeed * speedModifier; }
	float dashDuration = 0.0f;
	float DashDuration() { return dashDuration / speedModifier; }
	float dashCooldown = 0.0f;
	float DashCooldown() { return dashCooldown / cooldownModifier; }
	float dashDeccelerationRatio = 2.0f;

	// Rest
	float restDuration = 0.0f;
	float RestDuration() { return restDuration / attackSpeedModifier; }

	// Animations
	AnimationInfo walkAnimation = { "Walk" };
	AnimationInfo chargeAnimation = { "Charge" };
	AnimationInfo dashAnimation = { "Dash" };
	AnimationInfo restAnimation = { "Rest" };

	// Audio
	C_AudioSource* chargeAudio = nullptr;
	C_AudioSource* deathAudio = nullptr;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	BlurrgState state = BlurrgState::WANDER;

	// Movement
	void Wander();
	void Chase();

	float distance = 0.0f;
	float2 moveDirection = float2::zero;

	GameObject* player = nullptr;

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