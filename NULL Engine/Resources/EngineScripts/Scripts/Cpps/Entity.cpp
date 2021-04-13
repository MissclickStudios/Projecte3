#include "Entity.h"

Entity::Entity() : Script()
{
}

Entity::~Entity()
{
}

void Entity::Awake()
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
		switch (effects[i])
		{
		case Effect::FROZEN:
			Frozen();
			break;
		}
}

void Entity::Update()
{
}

void Entity::PostUpdate()
{
}

void Entity::CleanUp()
{
}

void Entity::Frozen()
{
}
