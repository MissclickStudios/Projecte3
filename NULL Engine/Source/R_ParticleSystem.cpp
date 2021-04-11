#include "JSONParser.h"

#include "FileSystemDefinitions.h"

#include "ParticleModule.h"

#include "R_Material.h"

#include "R_ParticleSystem.h"

#include "MemoryManager.h"

R_ParticleSystem::R_ParticleSystem() : Resource(ResourceType::PARTICLES)
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
	
	// REMEMBER TO GENERATE AN ASSET FILE FOR THIS

	for (auto emit = emitters.begin(); emit != emitters.end(); emit++)
	{
		/*if (emit->emitterTexture->GetUID() != 0)
		{
			std::string material_name = emit->emitterTexture->GetUID() + MATERIALS_EXTENSION;
			std::string material_path = MATERIALS_PATH + std::to_string(emit->emitterTexture->GetUID()) + MATERIALS_EXTENSION;

			ParsonNode material_node = contained_array.SetNode(material_name.c_str());

			material_node.SetNumber("UID", emit->emitterTexture->GetUID());
			material_node.SetNumber("Type", (uint)ResourceType::MATERIAL);
			material_node.SetString("Name", material_name.c_str());
			material_node.SetString("LibraryPath", material_path.c_str());
		}*/
	}

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
