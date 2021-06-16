#pragma once
#include "ScriptMacros.h"
#include "Script.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Algorithm/Random/LCG.h"

class GameManager;
class C_Canvas;
class C_UI_Image;
class CameraMovement;
class C_AudioSource;
class C_BoxCollider;

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
	BOMBING_ATTACK_IN,
	BOMBING_ATTACK,
	BOMBING_AND_SPIRAL_ATTACK_IN,
	BOMBING_AND_SPIRAL_ATTACK,
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
	void TakeDamage(float damage)override;

	void EntityPause() override;
	void EntityResume() override;

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
	AnimationInfo bombingAnimation = { "BombingAttack" };

	// Attack
	float attackDistance = 0.0f;
	float userAttackDistance = 0.0f;

	float firstStageAttackCooldown = 0.0f;
	float secondStageAttackCooldown = 0.0f;

	// Special Attack
	float spiralAttackDuration = 0.0f;
	float spiralAttackSpeed = 0.0f;
	float spiralAttackSpins = 0.0f;
	float spiralAttackHp = 0.0f;

	float lineAttackDuration = 0.0f;
	float lineAttackSpeed = 0.0f;
	float lineAttackSpins = 0.0f;
	float lineAttackShots = 0.0f;
	float lineAttackHp = 0.0f;

	float bombingAttackDuration = 0.0f;
	float bombingAttackSpeed = 0.0f;
	float bombingAttackHp = 0.0f;
	float bombingAttackShots = 0.0f;
	float bombingAttackBigAreaSide = 0.0f;
	float bombingAttackSmallAreaSide = 0.0f;
	float bombFallingTime = 0.0f;
	float prepareBombingTime = 0.0f;

	// Weapons
	Prefab blaster;
	Prefab sniper;
	Prefab bomb;
	Prefab bombProjectile;

	std::string rightHandName;
	std::string leftHandName;

	float minCredits = 0.f;
	float maxCredits = 0.f;

	GameObject* healthBarCanvasObject = nullptr;
	std::string lifeBarImageStr = "BossLife";
	std::string bossIconStr = "BossIcon";
	
	//partiles and SFX
	C_ParticleSystem* bombingParticles;
	C_ParticleSystem* hitParticles;
	bool bombExploding = false;


	int beskarValue = 4;

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
	bool BombingAttack();
	bool BombingAndSpiralAttack();

	void pickFirstStageAttack();		//Randomly picks an attack from the first stage
	void pickSecondStageAttack();		//Randomly picks an attack from the second stage

	float3 CalculateNextBomb(float x, float y);

	Timer firstStageTimer;
	Timer secondStageTimer;
	Timer spiralAttackTimer;
	Timer lineAttackTimer;
	Timer bombingAttackTimer;
	Timer bombTimer;
	Timer bombingAndSpiralAttackTimer;
	Timer prepareBombing;
	
	float2 bombPosition = float2::zero;
	float2 playerPosition = float2::zero;

	GameObject* crosshair = nullptr;

	float2 specialAttackStartAim = float2::zero;
	float specialAttackRot = 0.0f;
	//Bomb

	GameObject* bombGameObject = nullptr;
	C_BoxCollider* bombCollider = nullptr;

	GameObject* projectileGameObject = nullptr;
	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;

	GameObject* sniperGameObject = nullptr;
	Weapon* sniperWeapon = nullptr;

	LCG randomGenerator;

	GameManager* gameManager = nullptr;
	CameraMovement* cameraMovement = nullptr;

	C_Canvas* healthBarCanvas = nullptr;
	C_UI_Image* healthBarImage = nullptr;
	C_UI_Image* bossIcon = nullptr;
	float healthMaxW = 0.0f;

	//Audio
	C_AudioSource* deathAudio = nullptr;
	C_AudioSource* bombAudio = nullptr;
};

SCRIPTS_FUNCTION IG12* CreateIG12();
