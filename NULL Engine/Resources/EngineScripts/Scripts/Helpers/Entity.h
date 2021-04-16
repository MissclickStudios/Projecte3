#pragma once
#include "Script.h"

#include "EntityHelpers.h"

#include <string>
#include <vector>

#define DEFAULT_MODIFIER 1.0f

typedef unsigned int uint;

class C_RigidBody;
class C_Animator;

class Entity : public Script
{
public:

	Entity();
	virtual ~Entity();
	
	void Start();
	virtual void SetUp() = 0;
	
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
	float MaxHealth() { maxHealth + maxHealthModifier; }
	
	// Basic Stats
	float speed = 0.0f;
	const float Speed() const { return speed * speedModifier; }
	float attackSpeed = 0.0f;
	const float AttackSpeed() const { return attackSpeed * attackSpeedModifier; }
	float damage = 0.0f;
	const float Damage() const { return damage * damageModifier; }
	float defense = 0.0f;
	const float Defense() const { return defense * defenseModifier; }
	
	// Modifiers
	float maxHealthModifier = 0.0f;
	float speedModifier = DEFAULT_MODIFIER;
	float attackSpeedModifier = DEFAULT_MODIFIER;
	float damageModifier = DEFAULT_MODIFIER;
	float defenseModifier = DEFAULT_MODIFIER;
	float cooldownModifier = DEFAULT_MODIFIER;

	// Death
	float deathDuration = 0.0f; // Time between the entity dies and it gets removed
	
	// Basic Animations
	AnimationInfo idleAnimation = { "Idle" };
	AnimationInfo deathAnimation = { "Death" };

protected:

	// Movement
	C_RigidBody* rigidBody = nullptr;

	// Animations
	C_Animator* animator = nullptr;
	AnimationInfo* currentAnimation = nullptr;

	// Death
	Timer deathTimer;

private:

	// Effects
	std::vector<Effect*> effects;
	uint effectCounters[(uint)EffectType::EFFECTS_NUM];
};

//Entity Inspector varialbles
//// Health
//INSPECTOR_DRAGABLE_FLOAT(script->health);
//INSPECTOR_DRAGABLE_FLOAT(script->maxHealth);
//
//// Basic Stats
//INSPECTOR_DRAGABLE_FLOAT(script->speed);
//INSPECTOR_DRAGABLE_FLOAT(script->attackSpeed);
//INSPECTOR_DRAGABLE_FLOAT(script->damage);
//INSPECTOR_DRAGABLE_FLOAT(script->defense);
//
//// Modifiers
//INSPECTOR_DRAGABLE_FLOAT(script->maxHealthModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->cooldownModifier);
//
//// Death
//INSPECTOR_DRAGABLE_FLOAT(script->deathDuration);
//
//