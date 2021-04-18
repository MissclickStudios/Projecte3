#include "EmitterInstance.h"
#include "Application.h"
#include "M_Renderer3D.h"
#include "MemoryManager.h"
#include "Emitter.h"

#include "MathGeoLib/include/Math/float4x4.h"

EmitterInstance::EmitterInstance()
{
}

EmitterInstance::EmitterInstance(Emitter* emitter, C_ParticleSystem* component)
{
	this->emitter = emitter;
	this->component = component;
	emitterTime = 0.0f;
	activeParticles = 0;

	particles.resize(emitter->maxParticleCount);

	particleIndices = new unsigned int[emitter->maxParticleCount];

	for (uint i = 0; i < emitter->maxParticleCount; ++i)
	{
		particleIndices[i] = i;
	}
}

EmitterInstance::~EmitterInstance()
{
	delete[] particleIndices;	
}

void EmitterInstance::Init(Emitter* emitter, C_ParticleSystem* component)
{
	this->emitter = emitter;
	this->component = component;

	particles.resize(emitter->maxParticleCount);

	particleIndices = new unsigned int[emitter->maxParticleCount];

	for (uint i = 0; i < emitter->maxParticleCount; ++i)
	{
		particleIndices[i] = i;
	}
}

void EmitterInstance::Update(float dt)
{
	KillDeadParticles();
	UpdateModules(dt);
	DrawParticles();
}

void EmitterInstance::SpawnParticle()
{
	//check whether all particles are active or not
	//call the emitter reference to use the modules to spawn a particle. 
	//then add 1 to active particles.

	if (activeParticles == particles.size() || stopSpawn == true)
		return;

	unsigned int particleIndex = particleIndices[activeParticles];
	Particle* particle = &particles[particleIndex];

	for (unsigned int i = 0; i < emitter->modules.size(); i++)
	{
		emitter->modules[i]->Spawn(this, particle);
	}

	++activeParticles;
}

void EmitterInstance::ResetEmitter()
{
	emitterTime = 0.0f;
	activeParticles = 0; //kill all
}

void EmitterInstance::UpdateModules(float dt)
{
	for (int i = 0; i < emitter->modules.size(); i++)
	{
		emitter->modules[i]->Update(dt, this);
	}
}

void EmitterInstance::DrawParticles()
{
	//draw should ask the renderer to print a particle and give the details(transform, material, and color + distance to camera for sorting).
	for (int i = 0; i < activeParticles; i++)
	{
		unsigned int particleIndex = particleIndices[i];
		Particle* particle = &particles[particleIndex];

		float4x4 transform = float4x4::FromTRS(particle->position, particle->worldRotation, float3(particle->size, particle->size, particle->size)).Transposed();
		App->renderer->AddParticle(transform, emitter->emitterTexture, particle->color, particle->distanceToCamera);
	}
}

void EmitterInstance::KillDeadParticles()
{
	//loop through every active particles to see if they are still active. In case one is inactive,
	//swap the new dead particle with the last particle alive and subtract 1 to activeParticles.
	for (int i = (activeParticles - 1); i >= 0; --i)
	{
		unsigned int particleIndex = particleIndices[i];
		Particle* particle = &particles[particleIndex];

		if (particle->currentLifetime >= particle->maxLifetime)
		{
			if (i != (activeParticles - 1)) //if the last active particle is not active, skip the unnecesary changes
			{
				particleIndices[i] = particleIndices[activeParticles - 1];
				particleIndices[activeParticles - 1] = particleIndex;
			}

			--activeParticles;
		}
	}
}

void EmitterInstance::KillAll()
{
	activeParticles = 0;
}

