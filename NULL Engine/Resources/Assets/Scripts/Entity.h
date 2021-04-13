#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "EntityEnums.h"

#include <string>

#define DEFAULT_MODIFIER 1.0f

class SCRIPTS_API Entity : public Script 
{
public:

	Entity();
	virtual ~Entity();

	virtual void Awake();

	void PreUpdate();
	virtual void Update();
	void PostUpdate();

	virtual void CleanUp();

	// Effect Functions
	virtual void Frozen();

	// Type
	EntityType type = EntityType::ENTITY;

	// Health
	float health = 0.0f;
	float maxHealth = 0.0f;

	// Basic Stats
	float speed = 0.0f;
	float attackSpeed = 0.0f;
	float damage = 0.0f;
	float defense = 0.0f;

	// Modifiers
	float speedModifier = DEFAULT_MODIFIER;
	float attackSpeedModifier = DEFAULT_MODIFIER;
	float damageModifier = DEFAULT_MODIFIER;
	float defenseModifier = DEFAULT_MODIFIER;

	// Processed Stats
	float Speed() { return speed * speedModifier; }
	float AttackSpeed() { return attackSpeed * attackSpeedModifier; }
	float Damage() { return damage * damageModifier; }
	float Defense() { return defense * defenseModifier; }

	// Basic Animations
	std::string idle = "Idle";
	std::string death = "Death";

private:

	std::vector<Effect> effects;
};

SCRIPTS_FUNCTION Entity* CreateEntity() 
{
	Entity* script = new Entity();
	
	return script;
}

typedef unsigned int uint;