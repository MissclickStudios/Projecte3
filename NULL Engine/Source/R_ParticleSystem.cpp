#include "R_ParticleSystem.h"
#include "ParticleModule.h"
#include "MemoryManager.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLE_SYSTEM)
{
}

R_ParticleSystem::~R_ParticleSystem()
{
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
	emitters.clear();
	emitters.reserve(4);
	emitters.push_back(Emitter());

	emitters.back().modules.push_back(new EmitterBase);
	emitters.back().modules.push_back(new EmitterSpawn);
	emitters.back().modules.push_back(new ParticleColor);
	emitters.back().modules.push_back(new ParticleLifetime);
}
