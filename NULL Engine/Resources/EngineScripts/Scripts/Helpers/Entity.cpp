#include "Entity.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Animator.h"
#include "C_Material.h"
#include "C_AudioSource.h"

#include "C_ParticleSystem.h"
#include "Emitter.h"

#include "ScriptMacros.h"

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
	if (rigidBody && rigidBody->IsStatic())
		rigidBody = nullptr;
	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	// TODO: Particles with prefabs
	particles = gameObject->GetComponent<C_ParticleSystem>();
	if (particles)
	{
		particles->StopSpawn();

		for (uint i = 0; i < particles->emitterInstances.size(); ++i)
			if (particles->emitterInstances[i]->emitter->name == "Hit")
				hitParticles = particles->emitterInstances[i];
	}

	for (uint i = 0; i < gameObject->childs.size(); ++i)
	{
		std::string name = gameObject->childs[i]->GetName();
		if (name == "Skeleton")
		{
			skeleton = gameObject->childs[i];
			continue;
		}
		else if (gameObject->childs[i]->GetComponent<C_Material>())
		{
			material = gameObject->childs[i]->GetComponent<C_Material>();
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
	if (material)
		material->SetTakeDamage(false);

	// Set modifiers back to the default state
	maxHealthModifier = 0.0f;
	speedModifier = DEFAULT_MODIFIER;
	attackSpeedModifier = DEFAULT_MODIFIER;
	damageModifier = DEFAULT_MODIFIER;
	defenseModifier = DEFAULT_MODIFIER;
	cooldownModifier = DEFAULT_MODIFIER;

	// Loop through the Effects and call the respective functions
	for (uint i = 0; i < effects.size(); ++i)
	{
		if (effects[i]->IsActive()) // Check if the effect duration is ongoing
		{
			switch (effects[i]->Type()) // Call the corresponding function
			{
			case EffectType::FROZEN:
				Frozen();
				break;
			case EffectType::HEAL:
				Heal(effects[i]);
				break;
			}
		}
		else // Delete the effect if it ran out
		{
			--effectCounters[(uint)effects[i]->Type()]; // Substract one to the counter of this effect

			delete effects[i];
			effects.erase(effects.begin() + i);

			if (i <= 0) // Avoid relying on uints turning a high number to exit the loop when there are no more effects
				break;
			else
				--i;
		}
	}

	if (material && hitTimer.IsActive())
	{
		material->SetAlternateColour(Color(1, 0, 0, 1));
		material->SetTakeDamage(true);
		if (material && hitTimer.ReadSec() > hitDuration)
		{
			hitTimer.Stop();
			if (hitParticles)
				hitParticles->stopSpawn = true;
		}
	}
}

void Entity::PostUpdate()
{
	if (animator && currentAnimation)
	{
		AnimatorClip* clip = animator->GetCurrentClip();
		if (clip)
		{
			std::string clipName = clip->GetName();
			if (clipName != currentAnimation->name)	// If the animtion changed play the wanted clip
				animator->PlayClip(currentAnimation->name, currentAnimation->blendTime);
		}
		else
			animator->PlayClip(currentAnimation->name, currentAnimation->blendTime); // If there is no clip playing play the current animation
	}
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
	if (hitParticles)
		hitParticles->stopSpawn = false;

	if (damageAudio)
		damageAudio->PlayFx(damageAudio->GetEventId());
}

void Entity::GiveHeal(float amount)
{
	health += amount;
	if (health > MaxHealth())
		health = MaxHealth();
}

Effect* Entity::AddEffect(EffectType type, float duration, bool permanent)
{
	Effect* output = nullptr;
	// TODO: System to add a max stack to each effect so that more than one can exist at once

	if (effectCounters[(uint)type]) // Check that this effect is not already on the entity
		return output;
	
	output = new Effect(type, duration, permanent);
	effects.emplace_back(output); // I use emplace instead of push to avoid unnecessary copies
	++effectCounters[(uint)type]; // Add one to the counter of this effect

	return output;
}

void Entity::Frozen()
{
	speedModifier /= 2.5;
	attackSpeedModifier /= 2.5;

	if (material)
	{
		material->SetAlternateColour(Color(0, 1, 1, 1));
		material->SetTakeDamage(true);
	}
}

void Entity::Heal(Effect* effect)
{
	GiveHeal(effect->Duration());
	effect->End();
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

