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
	for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		//billboarding related things: world rotation and distance to camera.
	}
}

void EmitterSpawn::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void EmitterSpawn::Update(float dt, EmitterInstance* emitter)
{
	timer += dt; 
	if (timer >= spawnRatio)
	{
		timer = 0;
		emitter->SpawnParticle(); //SpawnParticle() will then call the Spawn() method in every particle module
	}
}

//void ParticlePosition::Spawn(EmitterInstance* emitter, Particle* particle)
//{
//	particle->position = initialPosition1;
//}
//
//void ParticlePosition::Update(float dt, EmitterInstance* emitter)
//{
//
//}

void ParticleColor::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->color = initialColor;
}

void ParticleColor::Update(float dt, EmitterInstance* emitter)
{
	//color over lifetime maybe??
	//should i add it here or in a new ParticleModule
}

void ParticleLifetime::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->maxLifetime = initialLifetime;
	particle->relativeLifetime = 0.0f;
}

void ParticleLifetime::Update(float dt, EmitterInstance* emitter)
{
	for (unsigned int i = 0; i < emitter->activeParticles; i++)
	{ 
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];
		
		particle->relativeLifetime += (1 / particle->maxLifetime) * dt;
		//when the relative lifetime equals or excedes 1.0f, the particle is killed by the emitter instance with KillDeadParticles
	}
}
