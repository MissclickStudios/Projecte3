#include "EmitterInstance.h"

EmitterInstance::EmitterInstance()
{
}

EmitterInstance::~EmitterInstance()
{
}

void EmitterInstance::Init(Emitter* emitter, C_ParticleSystem* component)
{
	this->emitter = emitter;
	this->component = component;

	particles.resize(emitter->maxParticleCount);
}

void EmitterInstance::Update(float dt)
{
	KillDeadParticles();
	UpdateModules(dt);
	DrawParticles();
}

void EmitterInstance::Spawn()
{
	//call the emitter reference to use the modules to spawn a particle. 
	//then add 1 to active particles.
}

void EmitterInstance::ResetEmitter()
{
	emitterTime = 0.0f;
	KillAll();
}

void EmitterInstance::UpdateModules(float dt)
{
	for (int i = 0; i < emitter->modules.size(); ++i)
	{
		emitter->modules[i]->Update(dt, this);
	}
}

void EmitterInstance::DrawParticles()
{
	//draw should ask the renderer to print a particle and give the details(transform, material, color,
	//distance to camera, and any other thing i consider necesary).

	//now just ask for a basic openGL square
}

void EmitterInstance::KillDeadParticles()
{
	//loop through every active particles to see if they are still active. In case one is inactive,
	//swap the new dead particle with the last particle alive and subtract 1 to activeParticles.
	for (int i = activeParticles - 1; i >= 0; --i)
	{
		unsigned int particleIndex = particleIndices[i];
		Particle* particle = &particles[particleIndex];

		if (particle->relativeLifetime >= 1.0f)
		{
			if (i =! activeParticles - 1) //if the last active particle is not active, skip the unnecesary changes
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
