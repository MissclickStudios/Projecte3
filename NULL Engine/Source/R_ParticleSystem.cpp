#include "R_ParticleSystem.h"

#include "MemoryManager.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLE_SYSTEM)
{
}

R_ParticleSystem::~R_ParticleSystem()
{
}

/*void R_ParticleSystem::InitDefaultSystem()
{
  
}*/

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
	
}
