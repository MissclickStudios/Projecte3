#pragma once
#include "Object.h"

#include "EntityHelpers.h"

#include <string>
#include <vector>
#include <unordered_map>

class GameObject;
class C_Mesh;
class C_Material;
class C_Animator;
class C_RigidBody;
class C_AudioSource;
class C_ParticleSystem;
class C_NavMeshAgent;

class AnimatorTrack;

enum class EntityType // jeje titties
{
	ENTITY,
	PLAYER,
	BLURRG,
	TROOPER,
	DARK_TROOPER,
	IG11,
	IG12,
	TURRET,
	GROGU
};

enum class EntityState
{
	NONE,
	STUNED,
	KNOCKEDBACK,
	ELECTROCUTED
};

typedef unsigned int uint;

#define DEFAULT_MODIFIER 1.0f

class Entity : public Object
{
public:

	Entity();
	virtual ~Entity();
	
	virtual void Awake();
	void Start();
	virtual void SetUp() = 0;
	
	void PreUpdate();
	void Update();
	virtual void Behavior() = 0;
	void PostUpdate();

	virtual void CleanUp() = 0;

	void OnPause() override;
	virtual void EntityPause() {}
	void OnResume() override;
	virtual void EntityResume() {}

	virtual void OnCollisionEnter(GameObject* object) override;

	// Interactions
	virtual void TakeDamage(float damage);
	virtual void GiveHeal(float amount);
	Effect* AddEffect(EffectType type, float duration, bool permanent = false, float power = 0.0f, float chance = 0.0f, float3 direction = float3::zero, bool start = true);
	virtual void ChangePosition(float3 position);
	bool IsGrounded();

	// Effect Functions
	virtual void Frozen(Effect* effect);
	virtual void Heal(Effect* effect);
	virtual void MaxHealthModify(Effect* effect);
	virtual void SpeedModify(Effect* effect);
	virtual void Stun(Effect* effect);
	virtual void KnockBack(Effect* effect);
	virtual void Electrocute(Effect* effect);
	virtual void BossPiercing(Effect* effect) {}
	virtual void PriceModify(Effect* effect);
	virtual void CooldownModify(Effect* effect);

	// Type
	EntityType type = EntityType::ENTITY;

	// Health
	float health = 1.0f;
	float maxHealth = 1.0f;
	float MaxHealth() { return maxHealth + maxHealthModifier; }

	// Basic Stats
	const float Speed() const		{ return speed * speedModifier; }
	const float AttackSpeed() const { return attackSpeed * attackSpeedModifier; }
	const float Damage() const		{ return damage * damageModifier; }
	const float Defense() const		{ return defense * defenseModifier; }
	
	float speed						= 0.0f;
	float attackSpeed				= 0.0f;
	float damage					= 0.0f;
	float defense					= 1.0f;

	// Modifiers
	float maxHealthModifier = 0.0f;
	float speedModifier = DEFAULT_MODIFIER;
	float attackSpeedModifier = DEFAULT_MODIFIER;
	float damageModifier = DEFAULT_MODIFIER;
	float defenseModifier = DEFAULT_MODIFIER;
	float cooldownModifier = DEFAULT_MODIFIER;
	float priceModifier = DEFAULT_MODIFIER;

	// Death
	float hitDuration = 0.25f;
	float deathDuration = 0.0f; // Time between the entity dies and it gets removed
	
	// Basic Animations
	GameObject* skeleton = nullptr;

	AnimationInfo idleAnimation			= { "Idle" };
	AnimationInfo deathAnimation		= { "Death" };
	AnimationInfo stunAnimation			= { "Stun" };
	AnimationInfo knockbackAnimation	= { "Knockback" };
	AnimationInfo electrocutedAnimation = { "Electrocuted" };

	Timer hitTimer;	

	// Audio
	C_AudioSource* walkAudio = nullptr;
	C_AudioSource* damageAudio = nullptr;

	// Agent 
	C_NavMeshAgent* agent = nullptr;

	// Hand
	std::string rightHandName;

	std::string leftHandName;

	// Particles
	std::vector<std::string> particleNames;

protected:

	// State
	EntityState GetEntityState();
	
	// Particles
	C_ParticleSystem* GetParticles(std::string particleName);

	// Movement
	C_RigidBody* rigidBody = nullptr;

	// Mesh
	C_Mesh* mesh = nullptr;

	// Animations
	C_Animator* animator = nullptr;
	AnimationInfo* currentAnimation = nullptr;

	//Material
	C_Material* material = nullptr;
	C_Material* secondaryMat = nullptr;

	// Death
	Timer deathTimer;

	// Effects
	std::vector<Effect*> effects;
	uint effectCounters[(uint)EffectType::EFFECTS_NUM];

	// Audio
	Timer stepTimer;

private:

	EntityState entityState = EntityState::NONE;

	// Particles
	std::unordered_map<std::string, C_ParticleSystem*> particles;

public:

	int dieAfterStun = 0;
};