#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"
#include <string>

class GameManager;
class C_Canvas;
class C_UI_Image;
class C_AudioSource;

enum class IG11State
{
	IDLE,
	PATROL,
	CHASE,
	FLEE,
	SPIRAL_ATTACK_IN,
	SPIRAL_ATTACK,
	U_ATTACK_IN,
	U_ATTACK,
	DOUBLE_SPIRAL_ATTACK_IN,
	DOUBLE_SPIRAL_ATTACK,
	ROTATE_ATTACK_IN,
	ROTATE_ATTACK,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API IG11 : public Entity ALLOWED_INHERITANCE
{
public:

	IG11();
	virtual ~IG11();

	void Start() override;
	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void EntityPause() override;
	void EntityResume() override;

	void OnCollisionEnter(GameObject* object) override;
	void TakeDamage(float damage)override;

	// Effects
	void BossPiercing(Effect* effect);

	// Movement
	std::string playerName = "Mando testbuild";

	// Chase
	float chaseDistance = 0.0f;
	float chaseSpeedModifier = DEFAULT_MODIFIER;
	float ChaseSpeed() { return speed * chaseSpeedModifier; }

	// Flee
	float fleeDistance = 0.0f;

	AnimationInfo talkAnimation = { "Talk" };
	AnimationInfo walkAnimation = { "Run" };
	AnimationInfo runAnimation = { "Run" };
	AnimationInfo fleeAnimation = { "Run" };
	AnimationInfo shootAnimation = { "Shoot" };
	AnimationInfo reloadAnimation = { "Reload" };
	AnimationInfo changeAnimation = { "Change" };
	AnimationInfo onGuardAnimation = { "OnGuard" };
	AnimationInfo specialAnimation = { "SpecialAttack" };
	AnimationInfo doubleSpecialAnimation = { "DoubleSpecial" };

	// Attack
	float attackDistance = 0.0f;

	// Special Attack
	float spiralAttackSpeed = 0.0f;
	float spiralAttackSpins = 0.0f;
	float spiralAttackHp = 0.0f;
	float spiralAttackCooldown = 0.0f;

	float UAttackShots = 0.0f;
	float UAttackCooldown = 0.0f;

	// Weapons
	Prefab blaster;
	Prefab sniper;

	std::string rightHandName;
	std::string leftHandName;

	float minCredits = 0.f;
	float maxCredits = 0.f;

	int beskarValue = 2;

	GameObject* healthBarCanvasObject = nullptr;
	std::string lifeBarImageStr = "BossLife";
	std::string bossIconStr = "BossIcon";

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	IG11State moveState = IG11State::PATROL;
	AimState aimState = AimState::IDLE;

	// Logic
	void ManageMovement();
	void ManageAim();

	IG11State FirstStageAttacks();
	IG11State SecondStageAttacks();

	// Movement
	void Patrol();
	void Chase();
	void Flee();

	float distance = 0.0f;
	float2 moveDirection = float2::zero;
	float2 aimDirection = float2::zero;
	float2 secondaryAimDirection = float2::zero;

	GameObject* player = nullptr;

	float baseFireRate = 0.0f;

	// Special Attack
	bool SpiralAttack();
	bool UAttack();
	bool DoubleSpiralAttack();
	bool RotateAttack();

	Timer spiralAttackTimer;
	Timer UAttackTimer;

	float2 specialAttackStartAim = float2::zero;
	float specialAttackRot = 0.0f;

	uint randomAttack = 0;
	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;

	GameObject* sniperGameObject = nullptr;
	Weapon* sniperWeapon = nullptr;

	//Hands
	GameObject* handLeft = nullptr;
	GameObject* handRight = nullptr;

	//float3 alternativeRight;
	//float3 alternativeLeft;
	//Game manager
	GameManager* gameManager = nullptr;

	C_Canvas* healthBarCanvas = nullptr;
	C_UI_Image* healthBarImage = nullptr;
	float healthMaxW = 0.0f;

	//Auido
	C_AudioSource* deathAudio = nullptr;
};

SCRIPTS_FUNCTION IG11* CreateIG11();