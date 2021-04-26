#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

enum class IG11State
{
	IDLE,
	PATROL,
	CHASE,
	FLEE,
	SPECIAL_ATTACK_IN,
	SPECIAL_ATTACK,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API IG11 : public Entity ALLOWED_INHERITANCE
{
public:

	IG11();
	virtual ~IG11();

	void SetUp() override;
	void Update() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	// Movement
	std::string playerName = "Mando testbuild";

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	float ChaseSpeed() { return speed * chaseSpeedModifier; }

	// Flee
	float fleeDistance = 0.0f;


	AnimationInfo walkAnimation = { "Run" };
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo fleeAnimation = { "Run" };
	AnimationInfo shootAnimation = { "Shoot" };
	AnimationInfo reloadAnimation = { "Reload" };
	AnimationInfo changeAnimation = { "Change" };
	AnimationInfo onGuardAnimation = { "OnGuard" };
	AnimationInfo specialAnimation = { "SpecialAttack" };

	// Attack
	float attackDistance = 0.0f;

	// Special Attack
	float specialAttackSpeed = 0.0f;
	float specialAttackSpins = 0.0f;
	float specialAttackHp = 0.0f;
	float specialAttackCooldown = 0.0f;

	// Weapons
	Prefab blaster;
	Prefab sniper;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	IG11State moveState = IG11State::PATROL;
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

	// Special Attack
	bool SpecialAttack(); // Rotation attack

	Timer specialAttackTimer;
	float2 specialAttackStartAim = float2::zero;
	float specialAttackRot = 0.0f;

	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;

	GameObject* sniperGameObject = nullptr;
	Weapon* sniperWeapon = nullptr;

};

SCRIPTS_FUNCTION IG11* CreateIG11();