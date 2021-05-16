#include "Entity.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_RigidBody.h"
#include "C_Animator.h"
#include "C_Material.h"
#include "C_AudioSource.h"
#include "C_Mesh.h"

#include "AnimatorTrack.h"

#include "C_ParticleSystem.h"
#include "Emitter.h"

#include "Random.h"

#include "ScriptMacros.h"

#include "MathGeoLib/include/Math/float3.h"
#include "CoreDllHelpers.h"

Entity::Entity() : Object()
{
	baseType = ObjectType::ENTITY;
}

Entity::~Entity()
{
	// Free possible residual or unfinished effects
	while (effects.size())
	{
		delete *effects.begin();
		effects.erase(effects.begin());
	}
	CoreCrossDllHelpers::CoreReleaseString(handName);
}

void Entity::Awake()
{
	rigidBody = gameObject->GetComponent<C_RigidBody>();
	if (rigidBody != nullptr && rigidBody->IsStatic())
		rigidBody = nullptr;
	animator = gameObject->GetComponent<C_Animator>();
	currentAnimation = &idleAnimation;

	for (uint i = 0; i < gameObject->childs.size(); ++i)
	{
		std::string name = gameObject->childs[i]->GetName();
		if (name == "Skeleton")
		{
			skeleton = gameObject->childs[i];
			continue;
		}
		else if (gameObject->childs[i]->GetComponent<C_Mesh>())
		{
			mesh = gameObject->childs[i]->GetComponent<C_Mesh>();
			material = gameObject->childs[i]->GetComponent<C_Material>();
		}
		else if (name == "Particles")
		{
			for (uint n = 0; n < gameObject->childs[i]->childs.size(); ++n)
			{
				C_ParticleSystem* particle = gameObject->childs[i]->childs[n]->GetComponent<C_ParticleSystem>();
				if (particle == nullptr)
					continue;
				std::string particleName = gameObject->childs[i]->childs[n]->GetName();

				particle->StopSpawn();

				particleNames.push_back(particleName); // For graphical representation porpouses
				particles.insert(std::make_pair(particleName, particle));
			}
		}
	}
	memset(effectCounters, 0, (uint)EffectType::EFFECTS_NUM); // Set all the counters to zero
}

void Entity::Start()
{
	deathTimer.Stop();
	hitTimer.Stop();
	stepTimer.Stop();

	SetUp();
}

void Entity::PreUpdate()
{
	if (material != nullptr)
		material->SetTakeDamage(false);

	// Set modifiers back to the default state
	maxHealthModifier = 0.0f;
	speedModifier = DEFAULT_MODIFIER;
	attackSpeedModifier = DEFAULT_MODIFIER;
	damageModifier = DEFAULT_MODIFIER;
	defenseModifier = DEFAULT_MODIFIER;
	cooldownModifier = DEFAULT_MODIFIER;
	entityState = EntityState::NONE;

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
			case EffectType::HEAL:
				Heal(effects[i]);
				break;
			case EffectType::MAX_HEALTH_MODIFY:
				MaxHealthModify(effects[i]);
				break;
			case EffectType::SPEED_MODIFY:
				SpeedModify(effects[i]);
				break;
			case EffectType::STUN:
				Stun(effects[i]);
				break;
			case EffectType::KNOCKBACK:
				KnockBack(effects[i]);
				break;
			}
		}
		else // Delete the effect if it ran out
		{
			--effectCounters[(uint)effects[i]->Type()]; // Substract one to the counter of this effect

			delete effects[i];
			effects.erase(effects.begin() + i);

			if (i <= 0) // Avoid relying on uints turning a high number to exit the loop when there are no more effects
				break;
			else
				--i;
		}
	}

	if (material != nullptr && hitTimer.IsActive())
	{
		material->SetAlternateColour(Color(1, 0, 0, 1));
		material->SetTakeDamage(true);
		if (hitTimer.ReadSec() > hitDuration)
		{
			hitTimer.Stop();
			if (GetParticles("Hit") != nullptr)
				GetParticles("Hit")->StopSpawn();
		}
	}
}

void Entity::Update()
{
	switch (entityState)
	{
	case EntityState::NONE:
		Behavior();
		break;
	case EntityState::STUNED:
		currentAnimation = &stunAnimation;
		break;
	}
}

void Entity::PostUpdate()
{
	if (animator != nullptr && currentAnimation != nullptr )
	{
		AnimatorClip* clip = animator->GetTrack("Preview").GetCurrentClip();

		if (clip == nullptr || clip->GetName() != currentAnimation->name)										// If no clip playing or animation/clip changed
			animator->PlayClip(currentAnimation->track.c_str(), currentAnimation->name.c_str(), currentAnimation->blendTime);

		/*if (clip != nullptr)
		{
			if (clip->GetName() != currentAnimation->name)	// If the animtion changed play the wanted clip
			{
				animator->PlayClip("Preview", currentAnimation->name.c_str(), currentAnimation->blendTime);
			}
		}
		else
		{
			animator->PlayClip("Preview", currentAnimation->name.c_str(), currentAnimation->blendTime); // If there is no clip playing play the current animation
		}*/
	}
}

void Entity::OnPause()
{
	deathTimer.Pause();
	stepTimer.Pause();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(false);

	EntityPause();
}

void Entity::OnResume()
{
	deathTimer.Resume();
	stepTimer.Resume();

	if (rigidBody != nullptr)
		rigidBody->SetIsActive(true);

	EntityResume();
}

void Entity::OnCollisionEnter(GameObject* object)
{
}

void Entity::TakeDamage(float damage)
{
	health -= damage / Defense();
	if (health < 0.0f)
		health = 0.0f;

	hitTimer.Start();
	if (GetParticles("Hit") != nullptr)
		GetParticles("Hit")->ResumeSpawn();

	if (damageAudio != nullptr)
		damageAudio->PlayFx(damageAudio->GetEventId());
}

void Entity::GiveHeal(float amount)
{
	health += amount;
	if (health > MaxHealth())
		health = MaxHealth();
}

Effect* Entity::AddEffect(EffectType type, float duration, bool permanent, void* data)
{
	// TODO: System to add a max stack to each effect so that more than one can exist at once
	if (effectCounters[(uint)type]) // Check that this effect is not already on the entity
	{
		if (data != nullptr)
			delete data;

		return nullptr;
	}
	
	Effect* output = new Effect(type, duration, permanent, data);
	effects.emplace_back(output); // I use emplace instead of push to avoid unnecessary copies
	++effectCounters[(uint)type]; // Add one to the counter of this effect

	return output;
}

void Entity::ChangePosition(float3 position)
{
	gameObject->transform->SetLocalPosition(position);
	rigidBody->TransformMovesRigidBody(true);
}

bool Entity::IsGrounded()
{
	if (rigidBody == nullptr)
		return false;

	if ((int)rigidBody->GetLinearVelocity().y == 0)
		return true;
	return false;
}

void Entity::Frozen()
{
	speedModifier /= 2.5;
	attackSpeedModifier /= 2.5;

	if (material != nullptr)
	{
		material->SetAlternateColour(Color(0, 1, 1, 1));
		material->SetTakeDamage(true);
	}
}

void Entity::Heal(Effect* effect)
{
	GiveHeal(effect->Duration());
	effect->End();
}

void Entity::MaxHealthModify(Effect* effect)
{
	maxHealthModifier += effect->Duration();
}

void Entity::SpeedModify(Effect* effect)
{
	speedModifier *= effect->Duration();
}

void Entity::Stun(Effect* effect)
{
	std::pair<bool, float>* data = (std::pair<bool, float>*)effect->Data();
	if (data && data->first)
	{
		data->first = false;

		float num = Random::LCG::GetBoundedRandomFloat(0, 100);
		if (num > data->second)
			effect->End();
	}
	else
		entityState = EntityState::STUNED;
}

void Entity::KnockBack(Effect* effect)
{
	std::pair<bool, float3>* data = (std::pair<bool, float3>*)effect->Data();
	if (data && data->first)
	{
		data->first = false;

		if (rigidBody != nullptr)
		{
			rigidBody->StopInertia();
			rigidBody->AddForce(data->second);
		}
	}
	entityState = EntityState::STUNED;
}

C_ParticleSystem* Entity::GetParticles(std::string particleName)
{
	bool found = false;
	for (uint i = 0; i < particleNames.size(); ++i)
		if (particleNames[i] == particleName)
		{
			found = true;
			break;
		}

	if (found)
		return particles.find(particleName)->second;
	return nullptr;
}

//	// Health
//	INSPECTOR_DRAGABLE_FLOAT(health);
//	INSPECTOR_DRAGABLE_FLOAT(maxHealth);
//
//	// Basic Stats
//	INSPECTOR_DRAGABLE_FLOAT(speed);
//	INSPECTOR_DRAGABLE_FLOAT(attackSpeed);
//	INSPECTOR_DRAGABLE_FLOAT(damage);
//	INSPECTOR_DRAGABLE_FLOAT(defense);
//
//	// Modifiers
//	INSPECTOR_DRAGABLE_FLOAT(maxHealthModifier);
//	INSPECTOR_DRAGABLE_FLOAT(speedModifier);
//	INSPECTOR_DRAGABLE_FLOAT(attackSpeedModifier);
//	INSPECTOR_DRAGABLE_FLOAT(damageModifier);
//	INSPECTOR_DRAGABLE_FLOAT(defenseModifier);
//	INSPECTOR_DRAGABLE_FLOAT(cooldownModifier);
//
//	// Death
//	INSPECTOR_DRAGABLE_FLOAT(deathDuration);

