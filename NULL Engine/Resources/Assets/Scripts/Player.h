#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

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

	void SetUp() override;
	void Update() override;
	void CleanUp() override;

	// Interactions
	void TakeDamage(float damage) override;

	// Dash
	float dashSpeed = 0.0f;
	float DashSpeed() { return dashSpeed * speedModifier; }
	float dashDuration = 0.0f;
	float DashDuration() { return dashDuration / speedModifier; }
	float dashCooldown = 0.0f;
	float DashCooldown() { return dashCooldown / cooldownModifier; }

	// Invencibility frames
	float invencibilityDuration = 0.0f;

	// Animations
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo dashAnimation = { "Dash" };
	AnimationInfo shootAnimation = { "Shoot", 0.05f };
	AnimationInfo reloadAnimation = { "Reload" };
	AnimationInfo changeAnimation = { "Change" };
	AnimationInfo onGuardAnimation = { "OnGuard" };

	// Weapons
	float changeTime = 0.0f;
	float ChangeTime() { return changeTime / attackSpeedModifier; }
	Prefab blaster;
	Prefab equipedGun;

	// Currency
	int currency = 0;

	std::string gameManager = "Game Manager";

private:

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

	// Aim
	void Aim();

	float2 aimDirection = float2::zero;

	// Dash
	void Dash();
	Timer dashTimer;
	Timer dashCooldownTimer;

	// Invencibility frames
	Timer invencibilityTimer;

	// Weapons
	Timer changeTimer;

	GameObject* blasterGameObject = nullptr;
	GameObject* equipedGunGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;
	Weapon* equipedGunWeapon = nullptr;
	Weapon* currentWeapon = nullptr;
};

SCRIPTS_FUNCTION Player* CreatePlayer();