#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"
#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

class GameObject;

class SCRIPTS_API Blurrg : public Script
{
public:

	Blurrg();
	~Blurrg();

	void Awake() override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void TakeDamage(float damage);
	void Freeze(float amount, float duration);
	void Weaken(float amount, float duration);

	// Movement
	float speed = 10.0f;

	float detectionRange = 40.0f;

	Prefab coin;
	GameObject* player = nullptr;

	// Dash
	float dashSpeed = 20.0f;
	float dashingTime = 0.4f;
	float dashingCharge = 1.0f;
	float dashingColdown = 3.0f;

	float dashRange = 20.0f;

	float dashRest = 2.0f;
	float restSpeed = 5.0f;

	// Health
	float health = 4.0f;
	float maxHealth = 4.0f;

	// Attack
	float damage = 0.5f;
	float dashDamage = 1.0f;

private:

	float3 LookingAt();

	// Movement
	float distance = 10000.0f;	// Distance from the player

	float3 direction = float3::zero;

	// Dash
	Timer dashTime; // Duration of the dash
	Timer dashColdown;
	Timer dashCharge;

	Timer restTimer;

	// States
	float speedModifier = 1;
	float attackModifier = 1;
	float defenseModifier = 1;

	float freezeDuration = 0.0f;
	Timer freezeTimer;
	float weakDuration = 0.0f;
	Timer weakTimer;

	GameObject* mesh = nullptr;
};


SCRIPTS_FUNCTION Blurrg* CreateBlurrg();