#include "Blurrg.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"

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
	INSPECTOR_DRAGABLE_FLOAT(script->maxHealthModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->cooldownModifier);

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
	if (state == BlurrgState::CHASE && player)
		LookAtPlayer();

	if (rigidBody)
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
			else if (distance < chargeDistance && dashCooldownTimer.ReadSec() >= DashCooldown())
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
			state = BlurrgState::CHARGE;

		case BlurrgState::CHARGE:
			if (chargeTimer.ReadSec() >= ChargeDuration())
			{
				chargeTimer.Stop();
				state = BlurrgState::DASH_IN;
			}
			break;
		case BlurrgState::DASH_IN:
			currentAnimation = &dashAnimation;
			chargeTimer.Start();
			state = BlurrgState::DASH;

		case BlurrgState::DASH:
			Dash();
			if (dashTimer.ReadSec() >= DashDuration())
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();
				state = BlurrgState::REST;
			}
			break;
		case BlurrgState::REST_IN:
			currentAnimation = &restAnimation;
			restTimer.Start();
			state = BlurrgState::REST;

		case BlurrgState::REST:
			if (restTimer.ReadSec() >= RestDuration())
			{
				restTimer.Stop();
				state = BlurrgState::WANDER;
			}
			break;
		case BlurrgState::DEAD_IN:
			currentAnimation = &deathAnimation;
			deathTimer.Start();
			state = BlurrgState::DEAD;

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
	health -= damage / Defense();
	if (health < 0.0f)
		health = 0.0f;
}

void Blurrg::Frozen()
{
}

void Blurrg::LookAtPlayer()
{
	float2 playerPosition, position;
	playerPosition.x = player->transform->GetWorldPosition().x;
	playerPosition.y = player->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;
	moveDirection = playerPosition - position;

	distance = moveDirection.Length();

	if (!moveDirection.IsZero())
		moveDirection.Normalize();

	float rad = moveDirection.AimedAngle();
}

void Blurrg::Wander()
{
	rigidBody->SetLinearVelocity(float3::zero);
}

void Blurrg::Chase()
{
	float3 direction = { moveDirection.x, 0.0f, moveDirection.y };
	rigidBody->SetLinearVelocity(direction * ChaseSpeed());
}

void Blurrg::Dash()
{
	float decceleration = (dashTimer.ReadSec() / DashDuration()) * 1.5f;
	if (decceleration > 1.0f)
		decceleration = 1.0f;

	float3 direction = { moveDirection.x, 0.0f, moveDirection.y };
	rigidBody->SetLinearVelocity(direction * DashSpeed() * decceleration);
}
