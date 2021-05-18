#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;
class C_2DAnimator;

enum class PlayerState
{
	IDLE,
	RUN,
	DASH_IN,
	DASH,
	DEAD_IN,
	DEAD,
	DEAD_OUT
};

class SCRIPTS_API Player : public Entity ALLOWED_INHERITANCE
{
public:

	Player();
	virtual ~Player();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void EntityPause() override;
	void EntityResume() override;

	void SaveState(ParsonNode& playerNode);
	void LoadState(ParsonNode& playerNode);

	void Reset();

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
	float intermitentMesh = 0.0f;

	// Animations
	void AnimatePlayer();
	AnimationInfo* GetMoveStateAnimation();
	AnimationInfo* GetAimStateAnimation();
	AnimationInfo* GetAimAnimation();
	AnimationInfo* GetShootAnimation();
	AnimationInfo* GetReloadAnimation();

	AnimatorTrack* torsoTrack	= nullptr;
	AnimatorTrack* legsTrack	= nullptr;

	AnimationInfo runAnimation				= { "Run" };
	AnimationInfo dashAnimation				= { "Dash" };

	AnimationInfo aimAnimation				= { "Aim" };
	AnimationInfo aimBlasterAnimation		= { "AimBlaster" };
	AnimationInfo aimSniperAnimation		= { "AimSniper" };
	AnimationInfo aimMinigunAnimation		= { "AimMinigun" };
	AnimationInfo aimShotgunAnimation		= { "AimShotgun" };

	AnimationInfo shootAnimation			= { "Shoot" };
	AnimationInfo shootBlasterAnimation		= { "ShootBlaster" };
	AnimationInfo shootSniperAnimation		= { "ShootSniper"};
	AnimationInfo shootMinigunAnimation		= { "ShootMinigun" };
	AnimationInfo shootShotgunAnimation		= { "ShootShotgun" };

	AnimationInfo reloadAnimation			= { "Reload" };
	AnimationInfo reloadBlasterAnimation	= { "ReloadBlaster" };
	AnimationInfo reloadSniperAnimation		= { "ReloadSniper" };
	AnimationInfo reloadMinigunAnimation	= { "ReloadMinigun" };
	AnimationInfo reloadShotgunAnimation	= { "ReloadShotgun" };

	AnimationInfo changeAnimation			= { "Change" };
	AnimationInfo onGuardAnimation			= { "OnGuard" };

	AnimationInfo torsoAnimation			= { "Idle", "Torso" };
	AnimationInfo legsAnimation				= { "Idle", "Legs" };

	// Weapons
	Weapon* const GetCurrentWeapon() const { return currentWeapon; }
	float changeTime = 0.0f;
	float ChangeTime() { return changeTime / attackSpeedModifier; }
	Prefab blaster;
	Prefab equipedGun;

	// Currency
	int currency = 0;
	int hubCurrency = 0;

	std::string gameManager = "Game Manager";

	PlayerState moveState = PlayerState::IDLE;
	AimState aimState = AimState::IDLE;

	// AIM HUD
	GameObject* idleAimPlane	= nullptr;
	GameObject* aimingAimPlane	= nullptr;

	// Debug
	void SetGodMode(bool enable);

	bool GetGodMode()const;

	// Audio
	C_AudioSource* dashAudio = nullptr;
	C_AudioSource* changeWeaponAudio = nullptr;
	C_AudioSource* deathAudio = nullptr;
	

	// HUD Animations Names
	std::string mandoImageName				= "Mando";
	std::string secondaryWeaponImageName	= "SecodaryWeapon";
	std::string primaryWeaponImageName		= "PrimaryWeapon";
	std::string dashImageName				= "Dash";
	std::string creditsImageName			= "Credits";

	//HUD Animations
	C_2DAnimator* mandoImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;

private:

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
	Timer intermitentMeshTimer;

	// Weapons
	bool usingEquipedGun = false;
	Timer changeTimer;

	GameObject* blasterGameObject		= nullptr;
	GameObject* equipedGunGameObject	= nullptr;

	Weapon* blasterWeapon				= nullptr;
	Weapon* secondaryWeapon				= nullptr;
	Weapon* currentWeapon				= nullptr;

	// Debug
	bool godMode = false;
};

SCRIPTS_FUNCTION Player* CreatePlayer();