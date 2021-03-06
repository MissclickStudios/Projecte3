#include "C_ParticleSystem.h"

#include "MemoryManager.h"

C_ParticleSystem::C_ParticleSystem(GameObject* owner) : Component(owner, ComponentType::TRANSFORM)
{
}

C_ParticleSystem::~C_ParticleSystem()
{
}

bool C_ParticleSystem::Update()
{
	return true;
}

void C_ParticleSystem::Reset()
{
}

