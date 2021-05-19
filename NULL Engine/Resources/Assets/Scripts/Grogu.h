#pragma once
#include "ScriptMacros.h"

#include "Timer.h"

#include "Entity.h"

#include "MathGeoLib/include/Math/float2.h"

class C_AudioSource;
class C_2DAnimator;
class C_BoxCollider;

class SCRIPTS_API Grogu : public Entity ALLOWED_INHERITANCE
{
public:

	Grogu();
	virtual ~Grogu();

	void SetUp() override;
	void Behavior() override;
	void CleanUp() override;

	//void OnTriggerRepeat(GameObject* object) override;

	void SaveState(ParsonNode& groguNode);
	void LoadState(ParsonNode& groguNode);

	void Reset();

	std::string gameManager = "Game Manager";

	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";

	float abilityCooldown = 0.0f;
	float AbilityCooldown() { return abilityCooldown / cooldownModifier; }

	C_BoxCollider* abilityCollider = nullptr;

private:

	// Logic
	void ManageMovement();
	void ManageRotation();
	void ManageAbility();

	// Actions
	void Movement();
	void Ability();

	float3 direction = float3::zero;

	Timer abilityCooldownTimer;

public:
	float maxDistanceToMando = 7.0f;
	float abilityPower = 2000000.0f;
	float abilityRadius = 0;

};

SCRIPTS_FUNCTION Grogu* CreateGrogu();