#include "ParticleModule.h"
#include "C_ParticleSystem.h"
#include "EmitterInstance.h"
#include "C_Transform.h"
#include "GameObject.h"

#include "MemoryManager.h"

void EmitterBase::Spawn(EmitterInstance* emitter, Particle* particle)
{
	GameObject* go = emitter->component->GetOwner();
	float3 position = go->GetComponent<C_Transform>()->GetWorldPosition();
	particle->position += position + origin;
}

void EmitterBase::Update(float dt, EmitterInstance* emitter)
{
	//update particles
}
// Alignment Rotation? 

void EmitterSpawn::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void EmitterSpawn::Update(float dt, EmitterInstance* emitter)
{

}

void ParticlePosition::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void ParticlePosition::Update(float dt, EmitterInstance* emitter)
{

}

void ParticleColor::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void ParticleColor::Update(float dt, EmitterInstance* emitter)
{

}
