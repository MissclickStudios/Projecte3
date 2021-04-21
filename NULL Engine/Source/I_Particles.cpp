#include "Application.h"
#include "JSONParser.h"
#include "R_ParticleSystem.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"

#include "I_Particles.h"

bool Importer::Particles::Import(const char* buffer, R_ParticleSystem* rParticles)
{
	// Import can reamain emtpy as Save and Load can handle everything.
	LOG("Importing particle system: %s", rParticles->GetAssetsPath());

	uint written = 0;

	ParsonNode particleSystemNode(buffer);
	rParticles->name = particleSystemNode.GetString("name");

	ParsonArray particleSystemArray = particleSystemNode.GetArray("Emitters");
	for (int i = 0; i < particleSystemArray.size; ++i)
	{
		Emitter emit;
		ParsonNode emitterNode = particleSystemArray.GetNode(i);
		emit.Load(emitterNode);
		rParticles->emitters.push_back(emit);
	}


	return true;
}

uint Importer::Particles::Save(R_ParticleSystem* rParticles, char** buffer) //Use the buffer to save the Asset I think
{
	uint written = 0;

	ParsonNode particleSystemNode;
	particleSystemNode.SetString("name", rParticles->name.c_str());

	ParsonArray particleSystemArray = particleSystemNode.SetArray("Emitters");
	for (std::vector<Emitter>::iterator emit = rParticles->emitters.begin(); emit != rParticles->emitters.end(); ++emit)
	{
		ParsonNode emitterNode = particleSystemArray.SetNode("emitter");
		emit->Save(emitterNode);
	}

	//std::string assetsPath = ASSETS_PARTICLESYSTEMS_PATH + std::to_string(rParticles->GetUID()) + PARTICLESYSTEMS_AST_EXTENSION;
	std::string assetsPath = ASSETS_PARTICLESYSTEMS_PATH + rParticles->name + PARTICLESYSTEMS_AST_EXTENSION;
	int size = particleSystemNode.SerializeToFile(assetsPath.c_str(), buffer);

	// --- SAVING THE BUFFER ---
	std::string libraryPath	= PARTICLESYSTEMS_PATH + std::to_string(rParticles->GetUID()) + PARTICLESYSTEMS_EXTENSION;
	char* libraryBuffer		= nullptr;
	written = particleSystemNode.SerializeToFile(libraryPath.c_str(), &libraryBuffer);
	
	RELEASE_ARRAY(libraryBuffer);
	
	//written = App->fileSystem->Save(path.c_str(), *buffer, size);

	if (written > 0)
	{
		LOG("[STATUS] ParticleSystem Importer: Successfully Saved ParticleSystem { %s } to Library! Path: %s", rParticles->GetAssetsFile(), libraryPath.c_str());
	}
	else
	{
		LOG("[ERROR] ParticleSystem Importer: Could not Save ParticleSystem { %s } to Library! Error: File System could not Write File.", rParticles->GetAssetsFile());
	}

	return written;
}

bool Importer::Particles::Load(const char* buffer, R_ParticleSystem* rParticles)
{
	uint written = 0;

	ParsonNode particleSystemNode(buffer);
	rParticles->name = particleSystemNode.GetString("name");

	ParsonArray particleSystemArray = particleSystemNode.GetArray("Emitters");
	for (int i = 0; i< particleSystemArray.size; ++i)
	{
		Emitter emit;
		ParsonNode emitterNode = particleSystemArray.GetNode(i);
		emit.Load(emitterNode);
		rParticles->emitters.push_back(emit);
	}

	return true;
}