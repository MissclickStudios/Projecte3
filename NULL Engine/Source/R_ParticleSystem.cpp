#include "R_ParticleSystem.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLE_SYSTEM)
{
}

R_ParticleSystem::~R_ParticleSystem()
{
}

void R_ParticleSystem::AddDefaultEmitter()
{
}

bool R_ParticleSystem::SaveMeta(ParsonNode& metaRoot) const
{
	return true;
}

bool R_ParticleSystem::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}
