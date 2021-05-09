#pragma once
#include "ScriptMacros.h"
#include "Script.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

enum class IG12State
{
	IDLE,
	PATROL,
	CHASE,
	FLEE,
	SPIRAL_ATTACK_IN,
	SPIRAL_ATTACK,
	LINE_ATTACK_IN,
	LINE_ATTACK,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API IG12 : public Entity ALLOWED_INHERITANCE
{
public:

	IG12();
	virtual ~IG12();

	void SetUp() override;
	void Behavior() override;
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
	float spiralAttackSpeed = 0.0f;
	float spiralAttackSpins = 0.0f;
	float spiralAttackHp = 0.0f;
	float spiralAttackCooldown = 0.0f;

	float lineAttackSpeed = 0.0f;
	float lineAttackSpins = 0.0f;
	float lineAttackHp = 0.0f;
	float lineAttackCooldown = 0.0f;
	float lineAttackShots = 0.0f;

	// Weapons
	Prefab blaster;
	Prefab sniper;

	std::string rightHandName;
	std::string leftHandName;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	IG12State moveState = IG12State::PATROL;
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
	float2 secondaryAimDirection = float2::zero;

	GameObject* player = nullptr;

	// Special Attack
	bool SpiralAttack();
	bool LineAttack();

	Timer spiralAttackTimer;
	Timer lineAttackTimer;

	float2 specialAttackStartAim = float2::zero;
	float specialAttackRot = 0.0f;

	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;

	GameObject* sniperGameObject = nullptr;
	Weapon* sniperWeapon = nullptr;

};

SCRIPTS_FUNCTION IG12* CreateIG12();
