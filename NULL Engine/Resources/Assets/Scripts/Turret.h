#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"
#include "AimStates.h"

#include "Blaster.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;

enum class TurretState
{
	IDLE,
	ATTACK,
	DEAD_IN,
	DEAD
};

class SCRIPTS_API Turret : public Entity ALLOWED_INHERITANCE
{
public:

	Turret();
	virtual ~Turret();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	// Movement
	std::string playerName = "Mando testbuild";

	// Attack
	float attackDistance = 0.0f;

	// Animations
	AnimationInfo shootAnimation = { "Shoot" };
	AnimationInfo reloadAnimation = { "Reload" };

	// Weapons
	Prefab blaster;

	// Audio
	C_AudioSource* deathAudio = nullptr;

private:

	void DistanceToPlayer();
	void LookAtPlayer();

	TurretState moveState = TurretState::IDLE;
	AimState aimState = AimState::IDLE;

	// Logic
	void ManageAim();

	float distance = 0.0f;
	float2 aimDirection = float2::zero;

	GameObject* player = nullptr;

	// Weapons
	GameObject* blasterGameObject = nullptr;
	Weapon* blasterWeapon = nullptr;
};

SCRIPTS_FUNCTION Turret* CreateTurret();