#include "Blurrg.h"
#include "Random.h"

#include "Application.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_AudioSource.h"
#include "C_NavMeshAgent.h"
#include "C_ParticleSystem.h"

#include "Player.h"

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
	INSPECTOR_STRING(script->playerName);

	// Charge
	INSPECTOR_DRAGABLE_FLOAT(script->chargeDistance);
	INSPECTOR_DRAGABLE_FLOAT(script->chargeDuration);

	// Dash
	INSPECTOR_DRAGABLE_FLOAT(script->dashDamageModifier);
	INSPECTOR_DRAGABLE_FLOAT(script->dashSpeed);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDuration);
	INSPECTOR_DRAGABLE_FLOAT(script->dashCooldown);
	INSPECTOR_DRAGABLE_FLOAT(script->dashDeccelerationRatio);

	// Rest
	INSPECTOR_DRAGABLE_FLOAT(script->restDuration);

	INSPECTOR_SLIDER_INT(script->minCredits, 0, 1000);
	INSPECTOR_SLIDER_INT(script->maxCredits, 0, 1000);

	//// Animations ---
	//// Movement
	//INSPECTOR_STRING(script->walkAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->walkAnimation.blendTime);
	//INSPECTOR_STRING(script->chargeAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->chargeAnimation.blendTime);
	//INSPECTOR_STRING(script->dashAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->dashAnimation.blendTime);
	//INSPECTOR_STRING(script->restAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->restAnimation.blendTime);
	//INSPECTOR_STRING(script->deathAnimation.name);
	//INSPECTOR_DRAGABLE_FLOAT(script->deathAnimation.blendTime);

	return script;
}

Blurrg::Blurrg() : Entity()
{
	type = EntityType::BLURRG;
}

Blurrg::~Blurrg()
{
}

void Blurrg::SetUp()
{
	chargeTimer.Stop();
	dashTimer.Stop();
	dashCooldownTimer.Stop();
	restTimer.Stop();

	player = App->scene->GetGameObjectByName(playerName.c_str());

	agent = gameObject->GetComponent<C_NavMeshAgent>();

	if (agent != nullptr)
	{
		agent->origin = gameObject->GetComponent<C_Transform>()->GetWorldPosition();
		agent->velocity = ChaseSpeed();
	}

	//Audios
	damageAudio = new C_AudioSource(gameObject);
	deathAudio = new C_AudioSource(gameObject);
	chargeAudio = new C_AudioSource(gameObject);
	walkAudio = new C_AudioSource(gameObject);
	if (damageAudio != nullptr)
		damageAudio->SetEvent("blurrg_damaged");
	if (deathAudio != nullptr)
		deathAudio->SetEvent("blurrg_death");
	if (chargeAudio != nullptr)
		chargeAudio->SetEvent("blurrg_charge");

	// Particles & SFX
	hitParticles = gameObject->GetComponent<C_ParticleSystem>();
	(hitParticles != nullptr) ? hitParticles->StopSpawn() : LOG("[ERROR] Blurg Script: Could not find { HIT } Particle System!");

	GameObject* chargeParticlesGO = gameObject->FindChild("Charge");
	chargeParticles = (chargeParticlesGO != nullptr) ? chargeParticlesGO->GetComponent<C_ParticleSystem>() : nullptr;
	
	(chargeParticles != nullptr) ? chargeParticles->StopSpawn() : LOG("[ERROR] Blurg Script: Could not find { CHARGE } Particle System!");
}

void Blurrg::Behavior()
{
	if (dieAfterStun == 2)
	{
		dieAfterStun = 3;
		state = BlurrgState::DEAD_IN;
		deathTimer.Resume();
	}
	if (state != BlurrgState::DEAD)
	{
		if (health <= 0.0f)
			state = BlurrgState::DEAD_IN;
		else
			if ((state == BlurrgState::WANDER || state == BlurrgState::CHASE) && player)
			{
				DistanceToPlayer();
				if (state == BlurrgState::CHASE)
					LookAtPlayer();
			}
	}

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
			else if (distance < chargeDistance && (dashCooldownTimer.ReadSec() >= DashCooldown() || !dashCooldownTimer.IsActive()))
			{
				state = BlurrgState::CHARGE_IN;
				break;
			}
			currentAnimation = &walkAnimation;
			Chase();
			break;
		case BlurrgState::CHARGE_IN:
			if (chargeAudio)
				chargeAudio->PlayFx(chargeAudio->GetEventId());

			currentAnimation = &chargeAnimation;
			if (agent != nullptr)
				agent->StopAndCancelDestination();
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
			if (agent != nullptr)
				agent->StopAndCancelDestination();
			dashTimer.Start();
			state = BlurrgState::DASH;

			if (chargeParticles != nullptr)
				chargeParticles->ResumeSpawn();

		case BlurrgState::DASH:
			Dash();
			if (dashTimer.ReadSec() >= DashDuration())
			{
				dashTimer.Stop();
				dashCooldownTimer.Start();
				state = BlurrgState::REST_IN;
				
				if (chargeParticles != nullptr)
					chargeParticles->StopSpawn();
			}
			break;
		case BlurrgState::REST_IN:
			currentAnimation = &restAnimation;
			if (agent != nullptr)
				agent->StopAndCancelDestination();
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
			if (deathAudio != nullptr)
				deathAudio->PlayFx(deathAudio->GetEventId());

			currentAnimation = &deathAnimation;
			if (rigidBody)
				rigidBody->SetIsActive(false); // Disable the rigidbody to avoid more interactions with other entities
			if (player)
			{
				Player* playerScript = (Player*)player->GetScript("Player");

				playerScript->GiveCredits(Random::LCG::GetBoundedRandomUint(minCredits, maxCredits));
			}
			deathTimer.Start();
			state = BlurrgState::DEAD;

		case BlurrgState::DEAD:
			if (dieAfterStun > 1)
				deathTimer.Resume();
			if (deathTimer.ReadSec() >= deathDuration)
				Deactivate();
			break;
		}
}

void Blurrg::CleanUp()
{

	if (damageAudio != nullptr)
		delete damageAudio;
	if (deathAudio != nullptr)
		delete deathAudio;
	if (chargeAudio != nullptr)
		delete chargeAudio;
	if (walkAudio != nullptr)
		delete walkAudio;
}

void Blurrg::OnCollisionEnter(GameObject* object)
{
	Player* playerScript = (Player*)object->GetScript("Player");
	if (playerScript)
	{
		switch (state)
		{
		case BlurrgState::DASH:
			playerScript->TakeDamage(DashDamage());
			break;
		default:
			playerScript->TakeDamage(Damage());
			break;
		}
	}
}

void Blurrg::EntityPause()
{
	chargeTimer.Pause();
	dashTimer.Pause();
	dashCooldownTimer.Pause();
	restTimer.Pause();

	if (agent != nullptr)
		agent->CancelDestination();
}

void Blurrg::EntityResume()
{
	chargeTimer.Resume();
	dashTimer.Resume();
	dashCooldownTimer.Resume();
	restTimer.Resume();
}

void Blurrg::DistanceToPlayer()
{
	if (!player)
		return;

	float2 playerPosition, position;
	playerPosition.x = player->transform->GetWorldPosition().x;
	playerPosition.y = player->transform->GetWorldPosition().z;
	position.x = gameObject->transform->GetWorldPosition().x;
	position.y = gameObject->transform->GetWorldPosition().z;

	moveDirection = playerPosition - position;

	distance = moveDirection.Length();

	if (!moveDirection.IsZero())
		moveDirection.Normalize();
}

void Blurrg::LookAtPlayer()
{
	float rad = moveDirection.AimedAngle();

	if (skeleton)
		skeleton->transform->SetLocalRotation(float3(0, -rad - DegToRad(90), 0));
}

void Blurrg::Wander()
{
	if (agent == nullptr)
		return;
	
	agent->StopAndCancelDestination();
}

void Blurrg::Chase()
{
	if (agent == nullptr)
		return;

	agent->velocity = ChaseSpeed();
	agent->SetDestination(player->transform->GetWorldPosition());

	walkAnimation.duration = 3.9f / speedModifier;
}

void Blurrg::Dash()
{
	float secondsToStop = DashDuration() - dashTimer.ReadSec(); // Don't want to explain this by text
	float decceleration = (secondsToStop / DashDuration()) * dashDeccelerationRatio; // if u have any doubts or questions ask me -> @David Rami
	if (decceleration > 1.0f)
		decceleration = 1.0f;

	if (agent != nullptr) {
		agent->velocity = DashSpeed() * decceleration;
		agent->SetDestination(agent->destinationPoint);
	}

}
