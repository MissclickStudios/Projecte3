#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

class GameObject;

class SCRIPTS_API SandTrooper : public Script
{
public:

	SandTrooper();
	~SandTrooper();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void TakeDamage(float damage);

	// Movement
	float speed = 10.0f;

	float detectionRange = 40.0f;

	GameObject* player = nullptr;

	// Health
	float health = 4.0f;
	float maxHealth = 4.0f;

	// Attack
	float damage = 0.5f;

private:

	float3 LookingAt();

	// Movement
	float distance = 10000.0f;	// Distance from the player

	float3 direction = float3::zero;
};


SCRIPTS_FUNCTION SandTrooper* CreateSandTrooper();