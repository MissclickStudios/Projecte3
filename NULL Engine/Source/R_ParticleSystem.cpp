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

bool R_ParticleSystem::SaveMeta(ParsonNode& metaRoot) const
{
	return false;
}

bool R_ParticleSystem::LoadMeta(const ParsonNode& metaRoot)
{
	return false;
}

void R_ParticleSystem::AddDefaultEmitter()
{
	//pushback an emitter and add it the standard particle modules
	emitters.push_back(Emitter());

	emitters.back().modules.push_back(new EmitterBase());
	emitters.back().modules.push_back(new ParticleColor());
	emitters.back().modules.push_back(new EmitterSpawn());
}

