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
	position += origin;
	particle->position = position;

	//temporary
	Quat rotation = go->GetComponent<C_Transform>()->GetWorldRotation();
	particle->worldRotation = rotation;
}

void EmitterBase::Update(float dt, EmitterInstance* emitter)
{
	//update particles
	for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		//update world rotation and distance to camera.
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
	//particles color over lifetime
	/*for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		if (particle->currentLifetime <= 0.25f)
			particle->color = Color(1.0f, 0.0f, 0.0f, 1.0f);

		else if (particle->currentLifetime > 0.25f && particle->currentLifetime <= 0.75)
			particle->color = Color(1.0f, 0.6f, 0.0f, 1.0f);

		else
			particle->color = Color(0.6f, 0.5f, 0.5f, 1.0f);
	}*/
}

void ParticleLifetime::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->maxLifetime = initialLifetime;
	particle->currentLifetime = 0.0f;
}

void ParticleLifetime::Update(float dt, EmitterInstance* emitter)
{
	for (unsigned int i = 0; i < emitter->activeParticles; i++)
	{ 
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];
		
		particle->currentLifetime += dt;

		//particle->currentLifetime += (1 / particle->maxLifetime) * dt;
		//when the relative lifetime equals or excedes 1.0f, the particle is killed by the emitter instance with KillDeadParticles()
	}
}
