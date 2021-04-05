#include "ParticleModule.h"
#include "C_ParticleSystem.h"
#include "Emitter.h"
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
	Quat rotation = Quat::FromEulerXYZ(90, 0, 0);
	particle->worldRotation = rotation;
}

void EmitterBase::Update(float dt, EmitterInstance* emitter)
{
	//update particles
	for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		//update distance to camera.
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

void ParticleMovement::Spawn(EmitterInstance* emitter, Particle* particle)
{
	float directionX = math::Lerp(initialDirection1.x, initialDirection2.x, randomGenerator.Float());
	float directionY = math::Lerp(initialDirection1.y, initialDirection2.y, randomGenerator.Float());
	float directionZ = math::Lerp(initialDirection1.z, initialDirection2.z, randomGenerator.Float());
	particle->movementDirection = float3(directionX, directionY, directionZ);

	particle->velocity = math::Lerp(initialIntensity1, initialIntensity2, randomGenerator.Float());
}

void ParticleMovement::Update(float dt, EmitterInstance* emitter)
{
	if (hideMovement == false)
	{
		for (unsigned int i = 0; i < emitter->activeParticles; i++)
		{
			unsigned int particleIndex = emitter->particleIndices[i];
			Particle* particle = &emitter->particles[particleIndex];

			particle->position += particle->movementDirection * particle->velocity * dt;
		
		}
	}
	if (eraseMovement == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::ParticleMovement);
	}
}

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
	if (eraseColor == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::ParticleColor);
	}
}

void ParticleLifetime::Spawn(EmitterInstance* emitter, Particle* particle)
{
	particle->maxLifetime = initialLifetime;
	particle->currentLifetime = 0.0f;
}

void ParticleLifetime::Update(float dt, EmitterInstance* emitter)
{
	if (hideLifetime == false) 
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
	if (eraseLifetime == true)
	{
		emitter->emitter->DeleteModuleFromType(ParticleModule::Type::ParticleLifetime);
	}
}
