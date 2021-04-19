#include "JSONParser.h"

#include "FileSystemDefinitions.h"

#include "ParticleModule.h"

#include "R_Material.h"

#include "R_ParticleSystem.h"

#include "MemoryManager.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLE_SYSTEM)
{

}

R_ParticleSystem::~R_ParticleSystem()
{

}

bool R_ParticleSystem::CleanUp()
{

	return true;
}

bool R_ParticleSystem::SaveMeta(ParsonNode& metaRoot) const
{
	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");


	return true;
}

bool R_ParticleSystem::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
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
