#include "R_Particles.h"
#include "ParticleModule.h"
#include "MemoryManager.h"

R_Particles::R_Particles() : Resource(ResourceType::PARTICLES)
{

}

R_Particles::~R_Particles()
{

}

bool R_Particles::CleanUp()
{
	return true;
}

bool R_Particles::SaveMeta(ParsonNode& metaRoot) const
{
	return true;
}

bool R_Particles::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}

void R_Particles::AddDefaultEmitter()
{
	emitters.clear();
	emitters.reserve(4);
	emitters.push_back(Emitter());

	emitters.back().modules.push_back(new EmitterBase);
	emitters.back().modules.push_back(new EmitterSpawn);
	emitters.back().modules.push_back(new ParticleColor);
	emitters.back().modules.push_back(new ParticleLifetime);
}