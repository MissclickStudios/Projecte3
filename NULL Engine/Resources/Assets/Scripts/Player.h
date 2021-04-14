#pragma once
#include "ScriptMacros.h"

#include "Entity.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float2.h"

class SCRIPTS_API Player : public Entity
{
public:

	Player();
	virtual ~Player();

	void Start();
	void Update();
	void CleanUp();

	// Interactions
	void TakeDamage(float damage);

	// Effects
	void Frozen();

	// Dash
	float dashSpeed = 0.0f;
	float dashDuration = 0.0f;
	float dashCooldown = 0.0f;

	// Animations
	std::string runAnimation = "Run";
	std::string dashAnimation = "Dash";
	std::string shootAnimation = "Shoot";

private:

	// Movement
	float2 moveDirection = float2::zero;
	float2 aimDirection = float2::zero;

	// Dash
	Timer dashTimer;
};

SCRIPTS_FUNCTION Player* CreatePlayer()
{
	Player* script = new Player();
	return script;
}