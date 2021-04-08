#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Timer.h"

#include "MathGeoLib/include/Math/float3.h"

class GameObject;

class SCRIPTS_API Blurrg : public Script
{
public:

	Blurrg();
	~Blurrg();

	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	// Movement
	float speed = 10.0f;

	float detectionRange = 40.0f;

	GameObject* player = nullptr;

	// Dash
	float dashSpeed = 20.0f;
	float dashingTime = 0.4f;
	float dashingCharge = 1.0f;
	float dashingColdown = 3.0f;

	float dashRange = 20.0f;

	// Health
	float health = 4.0f;
	float maxHealth = 4.0f;

private:

	float3 LookingAt();

	// Movement
	float distance = 10000.0f;	// Distance from the player

	float3 direction = float3::zero;

	// Dash
	Timer dashTime; // Duration of the dash
	Timer dashColdown;
	Timer dashCharge;
};


SCRIPTS_FUNCTION Blurrg* CreateBlurrg()
{
	Blurrg* script = new Blurrg();

	// Movement
	INSPECTOR_DRAGABLE_FLOAT(script->speed);

	INSPECTOR_DRAGABLE_FLOAT(script->detectionRange);

	//INSPECTOR_GAMEOBJECT(script->player);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingTime);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingCharge);
	INSPECTOR_DRAGABLE_FLOAT(script->dashingColdown);

	INSPECTOR_DRAGABLE_FLOAT(script->dashRange);

	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	return script;
}