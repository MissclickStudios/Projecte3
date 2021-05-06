#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;

enum class TrooperState
{
	IDLE,
	PATROL,
	CHASE,
	FLEE,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API Trooper : public Entity ALLOWED_INHERITANCE
{
public:

	Trooper();
	virtual ~Trooper();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void EntityPause() override;
	void EntityResume() override;

	void OnCollisionEnter(GameObject* object) override;

	// Movement
	std::string playerName = "Mando testbuild";

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	float ChaseSpeed() { return speed * chaseSpeedModifier; }

	// Flee
	float fleeDistance = 0.0f;

	// Attack
	float attackDistance = 0.0f;

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

	// Audio
	C_AudioSource* deathAudio = nullptr;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	TrooperState moveState = TrooperState::PATROL;
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
};

SCRIPTS_FUNCTION Trooper* CreateTrooper();