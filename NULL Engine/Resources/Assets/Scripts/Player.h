#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

#include "ItemRarity.h"

class C_AudioSource;
class C_2DAnimator;

struct ItemData;

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

enum class MoveDirection
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

enum class InteractionType
{
	NONE,
	TALK,
	USE,
	BUY,
	OPEN_CHEST,
	SIGNAL_GROGU
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

	// States
	PlayerState moveState	= PlayerState::IDLE;
	AimState aimState		= AimState::IDLE;

	void EnableInput();
	void DisableInput();

	// Interactions
	void TakeDamage(float damage) override;
	void SetPlayerInteraction(InteractionType type, float duration = 0.0f);		// If duration is 0.0f, then the duration will be set with the duration of the clip.

	// Movement
	float walkSpeed			= DEFAULT_MODIFIER;									// Speed at which the player will move the moment they enter the WALK State.
	float aimingSpeed		= DEFAULT_MODIFIER;									// Speed at which the player will move the moment they have any AimState other than AimState::NONE.

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
	
	AnimationInfo* legsMatrix[5][5]			= { nullptr };

	AnimatorTrack* torsoTrack				= nullptr;
	AnimatorTrack* legsTrack				= nullptr;

	AnimationInfo mainMenuAnimation			= { "MainMenu" };

	AnimationInfo walkAnimation				= { "Walk" };

	AnimationInfo runForwardsAnimation		= { "RunForwardsLight" };
	AnimationInfo runForwardsLightAnimation	= { "RunForwardsLight" };
	AnimationInfo runForwardsHeavyAnimation	= { "RunForwardsHeavy" };
	AnimationInfo runBackwardsAnimation		= { "RunBackwards" };
	AnimationInfo runLeftAnimation			= { "RunLeft" };
	AnimationInfo runRightAnimation			= { "RunRight" };

	AnimationInfo dashAnimation				= { "Dash" };

	AnimationInfo aimBlasterAnimation		= { "AimBlaster" };
	//AnimationInfo aimBlasterAnimation		= { "AimBlasterAlt" };
	AnimationInfo aimSniperAnimation		= { "AimSniper" };
	AnimationInfo aimMinigunAnimation		= { "AimMinigun" };
	AnimationInfo aimShotgunAnimation		= { "AimShotgun" };

	AnimationInfo shootBlasterAnimation		= { "ShootBlaster" };
	//AnimationInfo shootBlasterAnimation		= { "ShootBlasterAlt" };
	AnimationInfo shootSniperAnimation		= { "ShootSniper"};
	AnimationInfo shootMinigunAnimation		= { "ShootMinigun" };
	AnimationInfo shootShotgunAnimation		= { "ShootShotgun" };

	AnimationInfo reloadBlasterAnimation	= { "ReloadBlaster" };
	AnimationInfo reloadSniperAnimation		= { "ReloadSniper" };
	AnimationInfo reloadMinigunAnimation	= { "ReloadMinigun" }; 
	AnimationInfo reloadShotgunAnimation	= { "ReloadShotgun" };

	AnimationInfo stunnedAnimation			= { "Stun" };
	AnimationInfo knockedbackAnimation		= { "Knockback"};
	
	AnimationInfo talkAnimation				= { "Talk" };
	AnimationInfo useAnimation				= { "Use" };
	AnimationInfo openChestAnimation		= { "OpenChest" };
	AnimationInfo signalGroguAnimation		= { "SignalGrogu" };

	AnimationInfo changeWeaponAnimation		= { "ChangeWeapon" };
	AnimationInfo onGuardAnimation			= { "OnGuard" };

	// Weapons
	float ChangeTime()						{ return changeTime / attackSpeedModifier; }
	void EquipWeapon(Prefab weapon);
	Weapon* const GetCurrentWeapon() const	{ return currentWeapon; }
	Weapon* GetSecondaryWeapon() { return secondaryWeapon; };

	float changeTime = 0.0f;
	Prefab blaster;
	Prefab equipedGun;

	// Currency
	int currency				= 0;
	int hubCurrency				= 0;

	std::string gameManager		= "Game Manager";

	// AIM HUD
	GameObject* idleAimPlane	= nullptr;
	GameObject* aimingAimPlane	= nullptr;

	// Debug
	void SetGodMode(bool enable);
	bool GetGodMode()const;

	// Particles and SFX
	C_ParticleSystem* runParticles			= nullptr;
	C_ParticleSystem* dashParticles			= nullptr;

	C_AudioSource* dashAudio				= nullptr;
	C_AudioSource* deathAudio				= nullptr;
	C_AudioSource* changeWeaponAudio		= nullptr;
	
	// HUD Animations Names
	std::string mandoImageName				= "Mando";
	std::string secondaryWeaponImageName	= "SecodaryWeapon";
	std::string primaryWeaponImageName		= "PrimaryWeapon";
	std::string dashImageName				= "Dash";
	std::string creditsImageName			= "Credits";
	std::string beskarImageName				= "Beskar";

	//HUD Animations
	C_2DAnimator* mandoImage;
	C_2DAnimator* primaryWeaponImage;
	C_2DAnimator* secondaryWeaponImage;
	C_2DAnimator* dashImage;
	C_2DAnimator* creditsImage;
	C_2DAnimator* beskarImage;

	// Controller
	float joystickThreshold					= 25.0f;
	float joystickFactor					= 327.67;

	// Hands
	GameObject* rightHand					= nullptr;
	GameObject* leftHand					= nullptr;

	// Items
	void AddItem(ItemData* item);
	const std::vector<std::pair<bool, ItemData*>>* const GetItems() const { return &items; }

	//Die cutscene
	bool doDieCutscene = false;

private:
	// Inputs
	bool allowInput = true;

	// Set Up
	void SetUpLegsMatrix();

	// Logic
	void ManageInteractions();
	void ManageMovement();
	void ManageAim();
	void ManageInvincibility();

	// Interaction Methods
	void Use();
	void Buy();
	void Talk();
	void OpenChest();
	void SignalGrogu();
	
	Timer interactionTimer;
	float interactionDuration = 0.0f;

	// Movement Methods
	void MoveIdle();
	void Interact();
	void Walk();
	void Run();
	void DashIn();
	void Dash();
	void DeadIn();
	void Dead();

	// Aim Methods
	void AimIdle();
	void OnGuard();
	void Aiming();
	void ShootIn();
	void Shoot();
	void ReloadIn();
	void Reload();
	void ChangeIn();
	void Change();

	// Inputs & State Selection
	void GatherMoveInputs();
	void GatherAimInputs();
	void GatherInteractionInputs();

	void SetPlayerDirection();
	void SetAimDirection();

	MoveDirection moveDirection			= MoveDirection::NONE;
	AimDirection aimDirection			= AimDirection::NONE;
	InteractionType currentInteraction	= InteractionType::NONE;
	
	float2 moveInput					= float2::zero;
	float2 aimInput						= float2::zero;

	// Movement
	void Movement();

	float2 moveVector					= float2::zero;

	// Aim
	void Aim();

	float2 aimVector					= float2::zero;

	// Dash
	void ApplyDash();
	
	Timer dashTimer;
	Timer dashCooldownTimer;

	// Invencibility frames
	Timer invincibilityTimer;
	Timer intermitentMeshTimer;

	// Weapons
	bool usingSecondaryGun				= false;
	Timer changeTimer;

	GameObject* blasterBarrelTip		= nullptr;
	GameObject* sniperBarrelTip			= nullptr;
	GameObject* shotgunBarrelTip		= nullptr;
	GameObject* minigunBarrelTip		= nullptr;

	GameObject* blasterGameObject		= nullptr;
	GameObject* secondaryGunGameObject	= nullptr;

	Weapon* blasterWeapon				= nullptr;
	Weapon* secondaryWeapon				= nullptr;
	Weapon* currentWeapon				= nullptr;

	// Items
	void ApplyItems();

	std::vector<std::pair<bool, ItemData*>> items;
	std::vector<std::pair<bool, ItemData*>> savedItems;

	// Animations
	GameObject* hip		= nullptr;
	GameObject* torso	= nullptr;
	GameObject* legs	= nullptr;
	
	// Utilities
	float GetAnimatorClipDuration(const char* clipName);

	// Debug
	bool godMode = false;
};

SCRIPTS_FUNCTION Player* CreatePlayer();