#include "JSONParser.h"
#include "Profiler.h"

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
	for (auto emitter = emitters.begin(); emitter != emitters.end(); ++emitter)
	{
		(*emitter).CleanUp();
	}

	emitters.clear();

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

void R_ParticleSystem::AddNewEmitter()
{
	OPTICK_CATEGORY("C_Particle AddNewEmitter()", Optick::Category::Debug)
	emitters.push_back(Emitter());

	emitters.back().modules.push_back(new EmitterBase);
	emitters.back().modules.push_back(new EmitterSpawn);
	emitters.back().modules.push_back(new ParticleColor);
	emitters.back().modules.push_back(new ParticleLifetime);
}

void R_ParticleSystem::AddNewEmitter(const char* name)
{
	OPTICK_CATEGORY("C_Particle AddNewEmitter()", Optick::Category::Debug)
		emitters.push_back(Emitter(name));

	emitters.back().modules.push_back(new EmitterBase);
	emitters.back().modules.push_back(new EmitterSpawn);
	emitters.back().modules.push_back(new ParticleColor);
	emitters.back().modules.push_back(new ParticleLifetime);
}

//void R_ParticleSystem::DeleteEmitter()
//{
//
//}
