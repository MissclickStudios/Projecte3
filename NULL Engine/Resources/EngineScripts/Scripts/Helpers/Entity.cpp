#include "Entity.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Animator.h"
#include "C_ParticleSystem.h"
#include "C_Material.h"

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

	//hitParticle = gameObject->GetComponent<C_ParticleSystem>();
	//hitParticle->StopSpawn();

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

	if (hitTimer.ReadSec() > 0.2 &&  material)
	{
		if(material->GetTakeDamage()) material->SetTakeDamage(false);
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

	if (material)
	{
		material->SetAlternateColour(Color(1, 0, 0, 1));
		material->SetTakeDamage(true);
		hitTimer.Start();
	}
	
}

void Entity::GiveHeal(float amount)
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

