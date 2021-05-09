#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;

enum class DarkTrooperState
{
	IDLE,
	PATROL,
	CHASE,
	FLEE,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API DarkTrooper : public Entity ALLOWED_INHERITANCE
{
public:

	DarkTrooper();
	virtual ~DarkTrooper();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	// Movement
	std::string playerName = "Mandalorian";

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	float ChaseSpeed() { return speed * chaseSpeedModifier; }

	// Flee
	float fleeDistance = 0.0f;

	// Attack
	float attackDistance = 0.0f;

	float changeWeaponHealth = 0.0f;

	// Animations
	AnimationInfo walkAnimation = { "Run" };
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo fleeAnimation = { "Run" };
	AnimationInfo shootAnimation = { "Shoot" };
	AnimationInfo reloadAnimation = { "Reload" };
	AnimationInfo changeAnimation = { "Change" };
	AnimationInfo onGuardAnimation = { "OnGuard" };

	// Weapons
	Prefab blaster;
	Prefab shotgun;

	// Audio
	C_AudioSource* deathAudio = nullptr;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	DarkTrooperState moveState = DarkTrooperState::PATROL;
	AimState aimState = AimState::IDLE;

	// Logic
	void ManageMovement();
	void ManageAim();

	// Movement
	void Patrol();
	void Chase();
	void Flee();

	float distance = 0.0f;
	float2 moveDirection = float2::zero;
	float2 aimDirection = float2::zero;

	GameObject* player = nullptr;

	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;

	GameObject* shotgunGameObject = nullptr;
	Weapon* shotgunWeapon = nullptr;

	Weapon* currentWeapon = nullptr;
};

SCRIPTS_FUNCTION DarkTrooper* CreateDarkTrooper();