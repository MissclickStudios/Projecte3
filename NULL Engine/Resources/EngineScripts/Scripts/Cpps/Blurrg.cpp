#include "Blurrg.h"

Blurrg* CreateBlurrg()
{
	Blurrg* script = new Blurrg();

	// Entity ---
	// Health
	INSPECTOR_DRAGABLE_FLOAT(script->health);
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);

	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(script->speed);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->defense);

	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);

	// Death
	INSPECTOR_DRAGABLE_FLOAT(script->deathDuration);

	// Blurrg ---
	// Wander
	INSPECTOR_DRAGABLE_FLOAT(script->wanderRadius);

	// Chase
	INSPECTOR_DRAGABLE_FLOAT(script->chaseSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->chaseDistance);
	INSPECTOR_GAMEOBJECT(script->player);

	// Charge
	INSPECTOR_DRAGABLE_FLOAT(script->chargeDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chargeDuration);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->dashCooldown);

	// Rest
	INSPECTOR_DRAGABLE_FLOAT(script->restDuration);

	// Animations ---
	// Movement
	INSPECTOR_STRING(script->walkAnimation.name);
	INSPECTOR_DRAGABLE_FLOAT(script->walkAnimation.blendTime);
	INSPECTOR_STRING(script->chargeAnimation.name);
	INSPECTOR_DRAGABLE_FLOAT(script->chargeAnimation.blendTime);
	INSPECTOR_STRING(script->dashAnimation.name);
	INSPECTOR_DRAGABLE_FLOAT(script->dashAnimation.blendTime);
	INSPECTOR_STRING(script->restAnimation.name);
	INSPECTOR_DRAGABLE_FLOAT(script->restAnimation.blendTime);
	INSPECTOR_STRING(script->deathAnimation.name);
	INSPECTOR_DRAGABLE_FLOAT(script->deathAnimation.blendTime);

	return script;
}

Blurrg::Blurrg()
{
	chargeTimer.Stop();
	dashTimer.Stop();
	dashCooldownTimer.Stop();
	restTimer.Stop();
}

Blurrg::~Blurrg()
{
}

void Blurrg::SetUp()
{
}

void Blurrg::Update()
{
	switch (state)
	{
	case BlurrgState::WANDER:
		if (distance < chaseDistance)
		{
			state = BlurrgState::CHASE;
			break;
		}
		currentAnimation = &walkAnimation;
		Wander();
		break;
	case BlurrgState::CHASE:
		if (distance > chaseDistance)
		{
			state = BlurrgState::WANDER;
			break;
		}
		else if (distance < chargeDistance && dashCooldownTimer.ReadSec() >= dashCooldown)
		{
			state = BlurrgState::CHARGE_IN;
			break;
		}
		currentAnimation = &walkAnimation;
		Chase();
		break;
	case BlurrgState::CHARGE_IN:
		currentAnimation = &chargeAnimation;
		chargeTimer.Start();

	case BlurrgState::CHARGE:
		if (chargeTimer.ReadSec() >= chargeDuration)
		{
			chargeTimer.Stop();
			state = BlurrgState::DASH_IN;
		}
		break;
	case BlurrgState::DASH_IN:
		currentAnimation = &dashAnimation;
		chargeTimer.Start();

	case BlurrgState::DASH:
		Dash();
		if (dashTimer.ReadSec() >= dashDuration)
		{
			dashTimer.Stop();
			dashCooldownTimer.Start();
			state = BlurrgState::REST;
		}
		break;
	case BlurrgState::REST_IN:
		currentAnimation = &restAnimation;
		restTimer.Start();

	case BlurrgState::REST:
		if (restTimer.ReadSec() >= restDuration)
		{
			restTimer.Stop();
			state = BlurrgState::WANDER;
		}
		break;
	case BlurrgState::DEAD_IN:
		currentAnimation = &deathAnimation;
		deathTimer.Start();

	case BlurrgState::DEAD:
		Deactivate();
		break;
	}
}

void Blurrg::CleanUp()
{
}

void Blurrg::TakeDamage(float damage)
{
}

void Blurrg::Frozen()
{
}

void Blurrg::LookAtPlayer()
{
}

void Blurrg::Wander()
{
}

void Blurrg::Chase()
{
}

void Blurrg::Dash()
{
	// This will be called repeatedly during the dash so that u can deccelerate the dash to make it more natural... DUMBASS
}
