#include "Entity.h"

Entity::Entity() : Script()
{
}

Entity::~Entity()
{
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

void Entity::AddEffect(EffectType type, float duration)
{
	effects.emplace_back(new Effect(type, duration)); // I use emplace instead of push to avoid unnecessary copies
}