#include "Application.h"
#include "JSONParser.h"
#include "R_ParticleSystem.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"

#include "I_Particles.h"

bool Importer::Particles::Import(const char* buffer, R_ParticleSystem* rParticles)
{




	return true;
}

uint Importer::Particles::Save(R_ParticleSystem* rParticles, char** buffer) //Use the buffer to save the Asset I think
{
	uint written = 0;

	ParsonNode particleSystemNode;

	ParsonArray particleSystemArray = particleSystemNode.SetArray("Emitters");

	for (std::vector<Emitter>::iterator emit = rParticles->emitters.begin(); emit != rParticles->emitters.end(); ++emit)
	{
		ParsonNode emitterNode = particleSystemArray.SetNode("emitter");
		emit->Load(emitterNode);
	}

	std::string assetsPath = ASSETS_PARTICLESYSTEMS_PATH + std::to_string(rParticles->GetUID()) + PARTICLESYSTEMS_AST_EXTENSION;
	particleSystemNode.SerializeToFile(assetsPath.c_str(),buffer);

	int size = 0;
	// --- SAVING THE BUFFER ---

	std::string path = PARTICLESYSTEMS_PATH + std::to_string(rParticles->GetUID()) + PARTICLESYSTEMS_EXTENSION;
	written = App->fileSystem->Save(path.c_str(), *buffer, size);

	if (written > 0)
	{
		LOG("[STATUS] Animations Importer: Successfully Saved Animation { %s } to Library! Path: %s", rParticles->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Animations Importer: Could not Save Animation { %s } to Library! Error: File System could not Write File.", rParticles->GetAssetsFile());
	}

	return written;
}

bool Importer::Particles::Load(const char* buffer, R_ParticleSystem* rParticles)
{
	return true;
}