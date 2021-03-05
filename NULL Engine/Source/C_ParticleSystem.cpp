#include "C_ParticleSystem.h"

C_ParticleSystem::C_ParticleSystem(GameObject* owner) : Component(owner, ComponentType::TRANSFORM)
{
}

C_ParticleSystem::~C_ParticleSystem()
{
}

bool C_ParticleSystem::Update(float dt)
{
	//loop all emitters and trigger update() method

	return true;
}

void C_ParticleSystem::Reset()
{
	//loop all emitters and trigger reset() method
}

