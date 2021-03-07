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
	//loop every particle and trigger the method Update()
	//also set their rotation to the alignment rotation. Right now they just have to look straight 
} 

void EmitterSpawn::Spawn(EmitterInstance* emitter, Particle* particle)
{
	
}

void EmitterSpawn::Update(float dt, EmitterInstance* emitter)
{
	//add dt to current timer
	//check for timer to spawn particle
}

void ParticlePosition::Spawn(EmitterInstance* emitter, Particle* particle)
{
	//particle position +=  position
}

void ParticlePosition::Update(float dt, EmitterInstance* emitter)
{
	//--
}

void ParticleColor::Spawn(EmitterInstance* emitter, Particle* particle)
{
	//set a color for the particles
}

void ParticleColor::Update(float dt, EmitterInstance* emitter)
{
	//--
}
