#pragma once
#include "Script.h"

#include "EntityHelpers.h"

#include <string>
#include <vector>

#define DEFAULT_MODIFIER 1.0f

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

	// Death
	float deathDuration = 0.0f; // Time between the entity dies and it gets removed
	
	// Processed Stats
	const float Speed() const { return speed * speedModifier; }
	const float AttackSpeed() const { return attackSpeed * attackSpeedModifier; }
	const float Damage() const { return damage * damageModifier; }
	const float Defense() const { return defense * defenseModifier; }
	
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
};

typedef unsigned int uint;

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
//INSPECTOR_DRAGABLE_FLOAT(script->speedModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->attackSpeedModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->damageModifier);
//INSPECTOR_DRAGABLE_FLOAT(script->defenseModifier);
//
//// Death
//INSPECTOR_DRAGABLE_FLOAT(script->deathDuration);
//
//