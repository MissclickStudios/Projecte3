#include "Entity.h"
#include "Application.h"
#include "M_Scene.h"
#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_Animator.h"
#include "C_Material.h"
#include "C_AudioSource.h"
#include "C_Mesh.h"
#include "C_NavMeshAgent.h"

#include "AnimatorTrack.h"

#include "C_ParticleSystem.h"
#include "Emitter.h"

#include "Random.h"

#include "ScriptMacros.h"

#include "Player.h"

#include "MathGeoLib/include/Math/float3.h"

Entity::Entity() : Object()
{
	baseType = ObjectType::ENTITY;
}

Entity::~Entity()
{
	// Free possible residual or unfinished effects
	while (effects.size())
	{
		delete *effects.begin();
		effects.erase(effects.begin());
	}
}

void Entity::Awake()
{
	rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (rigidBody != nullptr && rigidBody->IsStatic())
		rigidBody = nullptr;
	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	if (type == EntityType::TURRET)
	{
		GameObject* turretHead = gameObject->FindChild("HeadMesh");
		if(turretHead)
			secondaryMat = turretHead->GetComponent<C_Material>();
	}

	for (uint i = 0; i < gameObject->childs.size(); ++i)
	{
		std::string name = gameObject->childs[i]->GetName();
		if (name == "Skeleton")
		{
			skeleton = gameObject->childs[i];
			continue;
		}
		else if (gameObject->childs[i]->GetComponent<C_Mesh>())
		{
			mesh = gameObject->childs[i]->GetComponent<C_Mesh>();
			material = gameObject->childs[i]->GetComponent<C_Material>();
			
		}
		else if (name == "Particles")
		{
			for (uint n = 0; n < gameObject->childs[i]->childs.size(); ++n)
			{
				C_ParticleSystem* particle = gameObject->childs[i]->childs[n]->GetComponent<C_ParticleSystem>();
				if (particle == nullptr)
					continue;
				std::string particleName = gameObject->childs[i]->childs[n]->GetName();

				particle->StopSpawn();

				particleNames.push_back(particleName); // For graphical representation porpouses
				particles.insert(std::make_pair(particleName, particle));
			}
		}
	}
	memset(effectCounters, 0, (uint)EffectType::EFFECTS_NUM); // Set all the counters to zero
}

void Entity::Start()
{
	deathTimer.Stop();
	hitTimer.Stop();
	stepTimer.Stop();

	SetUp();
}

void Entity::PreUpdate()
{
	if (paused)
		return;

	if (material != nullptr)
	{
		if(secondaryMat) secondaryMat->SetTakeDamage(false);
		material->SetTakeDamage(false);
	}

	// Set modifiers back to the default state
	maxHealthModifier = 0.0f;
	speedModifier = DEFAULT_MODIFIER;
	attackSpeedModifier = DEFAULT_MODIFIER;
	damageModifier = DEFAULT_MODIFIER;
	defenseModifier = DEFAULT_MODIFIER;
	cooldownModifier = DEFAULT_MODIFIER;
	priceModifier = DEFAULT_MODIFIER;
	entityState = EntityState::NONE;
	if (agent != nullptr)
		agent->velocity = Speed();
	
	// Loop through the Effects and call the respective functions
	for (uint i = 0; i < effects.size(); ++i)
	{
		if (effects[i]->IsActive())															// Check if the effect duration is ongoing
		{
			switch (effects[i]->Type())														// Call the corresponding function
			{
			case EffectType::FROZEN:			{ Frozen(effects[i]); }				break;
			case EffectType::HEAL:			    { Heal(effects[i]); }				break;
			case EffectType::MAX_HEALTH_MODIFY: { MaxHealthModify(effects[i]); }	break;
			case EffectType::SPEED_MODIFY:		{ SpeedModify(effects[i]); }		break;
			case EffectType::STUN:				{ Stun(effects[i]); }				break;
			case EffectType::KNOCKBACK:			{ KnockBack(effects[i]); }			break;
			case EffectType::ELECTROCUTE:		{ Electrocute(effects[i]); }		break;
			case EffectType::BOSS_PIERCING:		{ BossPiercing(effects[i]); }		break;
			case EffectType::PRICE_MODIFY:		{ PriceModify(effects[i]); }		break;
			case EffectType::COOLDOWN_MODIFY:
				CooldownModify(effects[i]); // oops
				break;
			}
		}
		else																				// Delete the effect if it ran out
		{
			--effectCounters[(uint)effects[i]->Type()];										// Substract one to the counter of this effect

			delete effects[i];
			effects.erase(effects.begin() + i);

			if (i <= 0)																		// Avoid relying on uints turning a high number to exit the loop when there are no more effects
				break;

			--i;
		}
	}

	if (material != nullptr && hitTimer.IsActive())
	{
		material->SetAlternateColour(Color(1, 0, 0, 1));
		material->SetTakeDamage(true);
		if (secondaryMat)
		{
			secondaryMat->SetAlternateColour(Color(1, 0, 0, 1));
			secondaryMat->SetTakeDamage(true);
		}

		if (hitTimer.ReadSec() > hitDuration)
		{
			hitTimer.Stop();
			if (GetParticles("Hit") != nullptr)
				GetParticles("Hit")->StopSpawn();
		}

		if ((type == EntityType::TURRET || type == EntityType::IG11 || type == EntityType::IG12) && rigidBody != nullptr)
			rigidBody->StopInertia();
	}
}

void Entity::Update()
{
	if (paused)
		return;

	if (gameObject->transform->GetLocalPosition().y < -1000)
		health = 0.0f;

	if (health <= 0.0f)
		if (type == EntityType::DARK_TROOPER || type == EntityType::TROOPER || type == EntityType::PLAYER)
			if (entityState != EntityState::NONE)
				deathTimer.Pause();
			else
				if (dieAfterStun == 0 || dieAfterStun == 1)
					++dieAfterStun;

	switch (entityState)
	{
	case EntityState::NONE:			{ Behavior(); }									break;
	case EntityState::STUNED:		{ currentAnimation = &stunAnimation; }			break;
	case EntityState::KNOCKEDBACK:	{ currentAnimation = &knockbackAnimation; }		break;
	case EntityState::ELECTROCUTED: { currentAnimation = &electrocutedAnimation; }	break;
	}
}

void Entity::PostUpdate()
{
	if (paused)
		return;

	if (animator != nullptr && currentAnimation != nullptr )
	{	
		AnimatorTrack* preview = animator->GetTrackAsPtr("Preview");
		
		if (preview == nullptr)
			return;

		if ((type != EntityType::PLAYER))
		{
			AnimatorClip* clip = preview->GetCurrentClip();

			if (clip == nullptr || clip->GetName() != currentAnimation->name)										// If no clip playing or animation/clip changed
			{
				if (currentAnimation->duration > 0.0f && clip != nullptr)
					clip->SetSpeed(clip->GetDurationInSeconds() / currentAnimation->duration);

				animator->PlayClip(currentAnimation->track.c_str(), currentAnimation->name.c_str(), currentAnimation->blendTime);
			}
		}
		else
		{
			((Player*)this)->AnimatePlayer();
		}
	}
}

void Entity::OnPause()
{
	paused = true;
	deathTimer.Pause();
	stepTimer.Pause();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(false);

	for (uint i = 0; i < effects.size(); ++i)
		effects[i]->Pause();

	if (animator != nullptr)
		animator->Pause(true);

	EntityPause();
}

void Entity::OnResume()
{
	paused = false;
	deathTimer.Resume();
	stepTimer.Resume();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(true);

	for (uint i = 0; i < effects.size(); ++i)
		effects[i]->Resume();

	if (animator != nullptr)
		animator->Play(true);

	EntityResume();
}

void Entity::OnCollisionEnter(GameObject* object)
{
}

void Entity::TakeDamage(float damage)
{
	health -= damage / Defense();
	if (health < 0.0f)
		health = 0.0f;

	hitTimer.Start();
	if (GetParticles("Hit") != nullptr)
		GetParticles("Hit")->ResumeSpawn();

	if (damageAudio != nullptr)
		damageAudio->PlayFx(damageAudio->GetEventId());
}

void Entity::GiveHeal(float amount)
{
	health += amount;
	if (health > MaxHealth())
		health = MaxHealth();
}

Effect* Entity::AddEffect(EffectType type, float duration, bool permanent, float power, float chance, float3 direction, bool start)
{
	if (effectCounters[(uint)type]) // Check that this effect is not already on the entity
	{
		if (type == EffectType::KNOCKBACK || type == EffectType::STUN)
			return nullptr;
		for (uint i = 0; i < effects.size(); ++i) // If it does erase the older one
			if (effects[i]->Type() == type)
				effects[i]->End();
	}
	
	Effect* output = new Effect(type, duration, permanent, power, chance, direction, start);
	effects.emplace_back(output);
	++effectCounters[(uint)type]; // Add one to the counter of this effect

	return output;
}

void Entity::ChangePosition(float3 position)
{
	gameObject->transform->SetLocalPosition(position);
	rigidBody->TransformMovesRigidBody(true);
}

bool Entity::IsGrounded()
{
	if (rigidBody == nullptr)
		return false;

	if ((int)rigidBody->GetLinearVelocity().y == 0)
		return true;
	return false;
}

void Entity::Frozen(Effect* effect)
{
	speedModifier *= effect->Power();
	if (agent != nullptr)
		agent->velocity = Speed();
	attackSpeedModifier *= effect->Power();

	if (material != nullptr)
	{
		material->SetAlternateColour(Color(0, 1, 1, 1));
		material->SetTakeDamage(true);
		if (secondaryMat)
		{
			secondaryMat->SetAlternateColour(Color(1, 0, 0, 1));
			secondaryMat->SetTakeDamage(true);
		}
	}
}

void Entity::Heal(Effect* effect)
{
	GiveHeal(effect->Duration());
	effect->End();
}

void Entity::MaxHealthModify(Effect* effect)
{
	maxHealthModifier += effect->Duration();
}

void Entity::SpeedModify(Effect* effect)
{
	speedModifier *= effect->Power();
}

void Entity::Stun(Effect* effect)
{
	if (effect->start)
	{
		effect->start = false;
	
		float num = Random::LCG::GetBoundedRandomFloat(0, 100);
		if (num > effect->Chance())
			effect->End();
		else if (walkAudio != nullptr)
		{
			walkAudio->SetEvent("moisture_active");
			walkAudio->PlayFx(walkAudio->GetEventId());
		}
	}
	else
	{
		if (rigidBody != nullptr)
			rigidBody->StopInertia();
		if (agent != nullptr)
			agent->CancelDestination();
		entityState = EntityState::ELECTROCUTED;
		electrocutedAnimation.duration = effect->Duration();
	}

	if (type == EntityType::PLAYER)
		((Player*)this)->ForceManageInvincibility();
}

void Entity::KnockBack(Effect* effect)
{
	if (agent != nullptr)
		agent->CancelDestination(true);

	if (effect->start)
	{
		effect->start = false;

		if (rigidBody != nullptr)
		{
			rigidBody->StopInertia();
			rigidBody->AddForce(effect->Direction());
		}

		knockbackAnimation.duration = effect->Duration();
	}

	if (type == EntityType::PLAYER)
		((Player*)this)->ForceManageInvincibility();

	entityState = EntityState::KNOCKEDBACK;
}

void Entity::Electrocute(Effect* effect)
{
	if (effect->start)
	{
		effect->start = false;
		electrocutedAnimation.duration = effect->Duration();

		if (walkAudio != nullptr)
		{
			walkAudio->SetEvent("moisture_active");
			walkAudio->PlayFx(walkAudio->GetEventId());
		}
	}

	if (rigidBody != nullptr)
		rigidBody->StopInertia();

	if (agent != nullptr)
		agent->CancelDestination();

	if (type == EntityType::PLAYER)
		((Player*)this)->ForceManageInvincibility();

	entityState = EntityState::ELECTROCUTED;
}

void Entity::PriceModify(Effect* effect)
{
	priceModifier *= effect->Power();
}

void Entity::CooldownModify(Effect* effect)
{
	cooldownModifier *= effect->Power();
}

EntityState Entity::GetEntityState()
{
	return entityState;
}

C_ParticleSystem* Entity::GetParticles(std::string particleName)
{
	bool found = false;
	for (uint i = 0; i < particleNames.size(); ++i)
		if (particleNames[i] == particleName)
		{
			found = true;
			break;
		}

	if (found)
		return particles.find(particleName)->second;
	return nullptr;
}

//	// Health
//	INSPECTOR_DRAGABLE_FLOAT(health);
//	INSPECTOR_DRAGABLE_FLOAT(maxHealth);
//
//	// Basic Stats
//	INSPECTOR_DRAGABLE_FLOAT(speed);
//	INSPECTOR_DRAGABLE_FLOAT(attackSpeed);
//	INSPECTOR_DRAGABLE_FLOAT(damage);
//	INSPECTOR_DRAGABLE_FLOAT(defense);
//
//	// Modifiers
//	INSPECTOR_DRAGABLE_FLOAT(maxHealthModifier);
//	INSPECTOR_DRAGABLE_FLOAT(speedModifier);
//	INSPECTOR_DRAGABLE_FLOAT(attackSpeedModifier);
//	INSPECTOR_DRAGABLE_FLOAT(damageModifier);
//	INSPECTOR_DRAGABLE_FLOAT(defenseModifier);
//	INSPECTOR_DRAGABLE_FLOAT(cooldownModifier);
//
//	// Death
//	INSPECTOR_DRAGABLE_FLOAT(deathDuration);

