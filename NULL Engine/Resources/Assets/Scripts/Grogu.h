#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;
class C_2DAnimator;

class SCRIPTS_API Grogu : public Entity ALLOWED_INHERITANCE
{
public:

	Grogu();
	virtual ~Grogu();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	void SaveState(ParsonNode& groguNode);
	void LoadState(ParsonNode& groguNode);

	void Reset();

	std::string gameManager = "Game Manager";

	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";

	float abilityCooldown = 0.0f;
	float AbilityCooldown() { return abilityCooldown / cooldownModifier; }

private:

	// Logic
	void ManageMovement();
	void ManageRotation();
	void ManageAbility();

	// Actions
	void Movement();
	void Ability();

	float3 position = float3::zero;
	float2 aimDirection = float2::zero;
	Timer abilityCooldownTimer;

public:
	float maxDistanceToMando = 7.0f;


};

SCRIPTS_FUNCTION Grogu* CreateGrogu();