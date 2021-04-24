#pragma once
#include "Object.h"

#include "EntityHelpers.h"

#include <string>
#include <vector>

#define DEFAULT_MODIFIER 1.0f

typedef unsigned int uint;

class GameObject;
class C_RigidBody;
class C_Animator;

enum class EntityType
{
	ENTITY,
	PLAYER,
	BLURRG,
	TROOPER,
	IG11
};

class Entity : public Object
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

	virtual void OnCollisionEnter(GameObject* object) override;
	
	void Deactivate();
	
	// Interactions
	virtual void TakeDamage(float damage);
	virtual void Heal(float amount);
	void AddEffect(EffectType type, float duration, bool permanent = false);
	
	// Effect Functions
	virtual void Frozen();
	
	// Type
	EntityType type = EntityType::ENTITY;
	
	// Health
	float health = 1.0f;
	float maxHealth = 1.0f;
	float MaxHealth() { return maxHealth + maxHealthModifier; }
	
	// Basic Stats
	float speed = 0.0f;
	const float Speed() const { return speed * speedModifier; }
	float attackSpeed = 0.0f;
	const float AttackSpeed() const { return attackSpeed * attackSpeedModifier; }
	float damage = 0.0f;
	const float Damage() const { return damage * damageModifier; }
	float defense = 1.0f;
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
	GameObject* skeleton = nullptr;

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