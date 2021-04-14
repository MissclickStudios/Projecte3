#include "Entity.h"

#include "GameObject.h"

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