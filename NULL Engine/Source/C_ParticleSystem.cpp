#include "C_ParticleSystem.h"

#include "MemoryManager.h"

C_ParticleSystem::C_ParticleSystem(GameObject* owner) : Component(owner, ComponentType::TRANSFORM)
{
}

C_ParticleSystem::~C_ParticleSystem()
{
}

bool C_ParticleSystem::Update(float dt)
{
	//loop all emitterInstances and trigger update() method
	for (unsigned int i = 0; i < emitterInstances.size(); i++)
	{
		emitterInstances[i].Update(dt);
	}

	return true;
}

void C_ParticleSystem::Reset()
{
	//loop all emitterInstances and trigger reset() methodç
	for (unsigned int i = 0; i < emitterInstances.size(); i++)
	{
		//emitterInstances[i].Reset();
	}
}

