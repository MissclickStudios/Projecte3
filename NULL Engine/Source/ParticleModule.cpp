#include "GameObject.h"
#include "JSONParser.h"
#include "C_Transform.h"
#include "C_Particles.h"

#include "Emitter.h"
#include "EmitterInstance.h"

#include "ParticleModule.h"

#include "MemoryManager.h"

void EmitterBase::Save(ParsonNode& node)
{
	node.SetInteger("Type",(int)type);
	node.SetFloat3("origin", origin);
}

void EmitterBase::Load(ParsonNode& node)
{
	origin = node.GetFloat3("origin");
}

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

void EmitterSpawn::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);

	node.SetNumber("spawnRatio", spawnRatio);
	node.SetNumber("timer", timer);
}

void EmitterSpawn::Load(ParsonNode& node)
{
	spawnRatio = node.GetNumber("spawnRatio");
	timer = node.GetNumber("timer");
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

void ParticleMovement::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);

	node.SetNumber("initialIntensity1",initialIntensity1);
	node.SetNumber("initialIntensity2", initialIntensity2);

	node.SetFloat3("initialDirection1",initialDirection1);
	node.SetFloat3("initialDirection2", initialDirection2);

	float3 initialPosition1 = float3::zero;
	float3 initialPosition2 = float3::zero;

	bool hideMovement = false;
	bool eraseMovement = false;
}

void ParticleMovement::Load(ParsonNode& node)
{

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

void ParticleColor::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);
}

void ParticleColor::Load(ParsonNode& node)
{

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

void ParticleLifetime::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);
}

void ParticleLifetime::Load(ParsonNode& node)
{

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

void ParticleBillboarding::Save(ParsonNode& node)
{
	node.SetInteger("Type", (int)type);
}

void ParticleBillboarding::Load(ParsonNode& node)
{

}

void ParticleBillboarding::Spawn(EmitterInstance* emitter, Particle* particle)
{

}

void ParticleBillboarding::Update(float dt, EmitterInstance* emitter)
{
	for (unsigned int i = 0; i < emitter->activeParticles; ++i)
	{
		unsigned int particleIndex = emitter->particleIndices[i];
		Particle* particle = &emitter->particles[particleIndex];

		//particle->worldRotation = GetAlignmentRotation(particle->position, )
	}
}

Quat ParticleBillboarding::GetAlignmentRotation(const float3& position, const float4x4& cameraTransform)
{
	return Quat(0, 0, 0, 0);	
}

