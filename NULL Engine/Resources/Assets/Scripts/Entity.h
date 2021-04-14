#pragma once
#include "Script.h"

#include "EntityHelpers.h"

#include <string>
#include <vector>

#define DEFAULT_MODIFIER 1.0f

class C_Animator;

class Entity : public Script
{
public:

	Entity();
	virtual ~Entity();

	virtual void Start() = 0;

	void PreUpdate();
	virtual void Update() = 0;
	void PostUpdate();

	virtual void CleanUp() = 0;

	void Deactivate();

	// Interactions
	virtual void TakeDamage(float damage) = 0;
	void AddEffect(EffectType type, float duration);

	// Effect Functions
	virtual void Frozen() = 0;

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
	 const float Speed() const { return speed * speedModifier; }
	 const float AttackSpeed() const { return attackSpeed * attackSpeedModifier; }
	 const float Damage() const { return damage * damageModifier; }
	 const float Defense() const { return defense * defenseModifier; }

	// Basic Animations
	std::string idleAnimation = "Idle";
	std::string deathAnimation = "Death";

protected:

	// Animations
	C_Animator* animator = nullptr;
		// Death
	float deathDuration = 0.0f; // Time between the entity dies and it gets removed
	Timer deathTimer;

private:

	// Effects
	std::vector<Effect*> effects;
};

typedef unsigned int uint;