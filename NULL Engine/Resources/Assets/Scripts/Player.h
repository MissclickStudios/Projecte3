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
	WALK,
	RUN,
	DASH_IN,
	DASH,
	DEAD_IN,
	DEAD,
	DEAD_OUT
};

enum class PlayerDirection
{
	NONE,
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

enum class AimDirection
{
	NONE,
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
};

enum class RunDirection
{
	NONE,
	FORWARDS,
	BACKWARDS,
	LEFT,
	RIGHT
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

public:

	void Reset();

	// Interactions
	void TakeDamage(float damage) override;

	// Movement
	float walkSpeed			= DEFAULT_MODIFIER;									// Factor by which the player's speed will be reduced when in WALK state.

	// Dash
	float DashSpeed()		{ return dashSpeed * speedModifier; }
	float DashDuration()	{ return dashDuration / speedModifier; }
	float DashCooldown()	{ return dashCooldown / cooldownModifier; }
	
	float dashSpeed			= 0.0f;
	float dashDuration		= 0.0f;
	float dashCooldown		= 0.0f;

	// Invencibility frames
	float invincibilityDuration = 0.0f;
	float intermitentMesh		= 0.0f;

	// Animations
	void AnimatePlayer();
	AnimationInfo* GetMoveStateAnimation();
	AnimationInfo* GetLegsAnimation();
	AnimationInfo* GetAimStateAnimation();
	AnimationInfo* GetAimAnimation();
	AnimationInfo* GetShootAnimation();
	AnimationInfo* GetReloadAnimation();

	AnimatorTrack* torsoTrack	= nullptr;
	AnimatorTrack* legsTrack	= nullptr;

	AnimationInfo mainMenuAnimation			= { "MainMenu" };

	AnimationInfo walkAnimation				= { "Walk" };

	AnimationInfo runAnimation				= { "Run" };
	AnimationInfo runForwardsAnimation		= { "RunForwards" };
	AnimationInfo runBackwardsAnimation		= { "RunBackwards" };
	AnimationInfo runLeftAnimation			= { "RunLeft" };
	AnimationInfo runRightAnimation			= { "RunRight" };

	AnimationInfo dashAnimation				= { "Dash" };

	AnimationInfo aimBlasterAnimation		= { "AimBlaster" };
	AnimationInfo aimSniperAnimation		= { "AimSniper" };
	AnimationInfo aimMinigunAnimation		= { "AimMinigun" };
	AnimationInfo aimShotgunAnimation		= { "AimShotgun" };

	AnimationInfo shootBlasterAnimation		= { "ShootBlaster" };
	AnimationInfo shootSniperAnimation		= { "ShootSniper"};
	AnimationInfo shootMinigunAnimation		= { "ShootMinigun" };
	AnimationInfo shootShotgunAnimation		= { "ShootShotgun" };

	AnimationInfo reloadBlasterAnimation	= { "ReloadBlaster" };
	AnimationInfo reloadSniperAnimation		= { "ReloadSniper" };
	AnimationInfo reloadMinigunAnimation	= { "ReloadMinigun" }; 
	AnimationInfo reloadShotgunAnimation	= { "ReloadShotgun" };

	AnimationInfo stunnedAnimation			= { "Stunned" };
	AnimationInfo knockedbackAnimation		= { "KnockedBack"};
	
	AnimationInfo talkAnimation				= { "Talk" };
	AnimationInfo useAnimation				= { "Use" };
	AnimationInfo openChestAnimation		= { "OpenChest" };
	AnimationInfo signalGroguAnimation		= { "SignalGrogu" };

	AnimationInfo changeWeaponAnimation		= { "ChangeWeapon" };
	AnimationInfo onGuardAnimation			= { "OnGuard" };

	// Weapons
	Weapon* const GetCurrentWeapon() const	{ return currentWeapon; }
	float ChangeTime()						{ return changeTime / attackSpeedModifier; }
	
	float changeTime = 0.0f;
	Prefab blaster;
	Prefab equipedGun;

	// Currency
	int currency				= 0;
	int hubCurrency				= 0;

	std::string gameManager		= "Game Manager";

	PlayerState moveState		= PlayerState::IDLE;
	AimState aimState			= AimState::IDLE;

	// AIM HUD
	GameObject* idleAimPlane	= nullptr;
	GameObject* aimingAimPlane	= nullptr;

	// Debug
	void SetGodMode(bool enable);
	bool GetGodMode()const;

	// Audio
	C_AudioSource* dashAudio				= nullptr;
	C_AudioSource* deathAudio				= nullptr;
	C_AudioSource* changeWeaponAudio		= nullptr;
	

	// HUD Animations Names
	std::string mandoImageName				= "Mando";
	std::string secondaryWeaponImageName	= "SecodaryWeapon";
	std::string primaryWeaponImageName		= "PrimaryWeapon";
	std::string dashImageName				= "Dash";
	std::string creditsImageName			= "Credits";

	//HUD Animations
	C_2DAnimator* mandoImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;

private:

	// Logic
	void ManageMovement();
	void ManageAim();

	// Inputs & State Selection
	void GatherMoveInputs();
	void GatherAimInputs();

	void SetPlayerDirection();
	void SetAimDirection();

	PlayerDirection playerDirection	= PlayerDirection::NONE;
	AimDirection aimDirection		= AimDirection::NONE;
	float2 moveInput				= float2::zero;
	float2 aimInput					= float2::zero;

	// Movement
	void Movement();

	float2 moveVector				= float2::zero;

	// Aim
	void Aim();

	float2 aimVector				= float2::zero;

	// Dash
	void Dash();
	
	Timer dashTimer;
	Timer dashCooldownTimer;

	// Invencibility frames
	Timer invencibilityTimer;
	Timer intermitentMeshTimer;

	// Weapons
	bool usingSecondaryGun				= false;
	Timer changeTimer;

	GameObject* blasterGameObject		= nullptr;
	GameObject* secondaryGunGameObject	= nullptr;

	Weapon* blasterWeapon				= nullptr;
	Weapon* secondaryWeapon				= nullptr;
	Weapon* currentWeapon				= nullptr;

	// Debug
	bool godMode = false;
};

SCRIPTS_FUNCTION Player* CreatePlayer();