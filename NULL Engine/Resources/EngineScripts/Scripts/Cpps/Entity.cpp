#include "Entity.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Animator.h"

#include "ScriptMacros.h"

Entity::Entity() : Script()
{
	deathTimer.Stop();
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

void Entity::Start()
{
	rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (rigidBody && rigidBody->IsStatic())
		rigidBody = nullptr;

	memset(effectCounters, 0, (uint)EffectType::EFFECTS_NUM); // Set all the counters to zero

	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	SetUp();
}

void Entity::PreUpdate()
{
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
			}
		}
		else // Delete the effect if it ran out
		{
			delete effects[i];
			effects.erase(effects.begin() + i);
			--i;
			--effectCounters[(uint)effects[i]->Type()]; // Substract one to the counter of this effect
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

void Entity::Deactivate()
{
	for (uint i = 0; i < gameObject->components.size(); ++i)
		gameObject->components[i]->SetIsActive(false);
	gameObject->SetIsActive(false);
}

void Entity::TakeDamage(float damage)
{
	health -= damage / Defense();
	if (health < 0.0f)
		health = 0.0f;
}

void Entity::Heal(float amount)
{
	health += amount;
	if (health > MaxHealth())
		health = MaxHealth();
}

void Entity::AddEffect(EffectType type, float duration, bool permanent)
{
	// TODO: System to add a max stack to each effect so that more than one can exist at once

	if (effectCounters[(uint)type]) // Check that this effect is not already on the entity
		return;
	effects.emplace_back(new Effect(type, duration, permanent)); // I use emplace instead of push to avoid unnecessary copies
	++effectCounters[(uint)type]; // Add one to the counter of this effect
}

void Entity::Frozen()
{
	speedModifier /= 2.5;
	attackSpeedModifier /= 2.5;
}

void Entity::InspectorCalls()
{
	// Health
	INSPECTOR_DRAGABLE_FLOAT(health);
	INSPECTOR_DRAGABLE_FLOAT(maxHealth);

	// Basic Stats
	INSPECTOR_DRAGABLE_FLOAT(speed);
	INSPECTOR_DRAGABLE_FLOAT(attackSpeed);
	INSPECTOR_DRAGABLE_FLOAT(damage);
	INSPECTOR_DRAGABLE_FLOAT(defense);

	// Modifiers
	INSPECTOR_DRAGABLE_FLOAT(maxHealthModifier);
	INSPECTOR_DRAGABLE_FLOAT(speedModifier);
	INSPECTOR_DRAGABLE_FLOAT(attackSpeedModifier);
	INSPECTOR_DRAGABLE_FLOAT(damageModifier);
	INSPECTOR_DRAGABLE_FLOAT(defenseModifier);
	INSPECTOR_DRAGABLE_FLOAT(cooldownModifier);

	// Death
	INSPECTOR_DRAGABLE_FLOAT(deathDuration);
}
