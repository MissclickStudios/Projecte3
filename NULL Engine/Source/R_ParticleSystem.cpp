#include "R_ParticleSystem.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLE_SYSTEM)
{
}

R_ParticleSystem::~R_ParticleSystem()
{
}

void R_ParticleSystem::InitDefaultSystem()
{
	emitters.clear();
	emitters.reserve(4);
	AddDefaultEmitter();
}

void R_ParticleSystem::AddDefaultEmitter()
{
	//pushback an emitter and add it the standard particle modules
}

