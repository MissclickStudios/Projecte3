#include "Entity.h"

#include "GameObject.h"
#include "C_RigidBody.h"
#include "C_Animator.h"

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

	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	SetUp();
}

void Entity::PreUpdate()
{
	// Set modifiers back to the default state
	speedModifier = DEFAULT_MODIFIER;
	attackSpeedModifier = DEFAULT_MODIFIER;
	damageModifier = DEFAULT_MODIFIER;
	defenseModifier = DEFAULT_MODIFIER;

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

void Entity::Deactivate()
{
	for (uint i = 0; i < gameObject->components.size(); ++i)
		gameObject->components[i]->SetIsActive(false);
	gameObject->SetIsActive(false);
}

void Entity::AddEffect(EffectType type, float duration)
{
	effects.emplace_back(new Effect(type, duration)); // I use emplace instead of push to avoid unnecessary copies
}