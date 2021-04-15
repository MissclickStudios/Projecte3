#include "JSONParser.h"
#include "Application.h"
#include "M_ResourceManager.h"

#include "ParticleModule.h"

#include "R_Texture.h"

#include "Emitter.h"

#include "MemoryManager.h"

Emitter::Emitter()
{
	//std::vector<R_Texture*> textures;
	//App->resourceManager->GetAllTextures(textures);
	//emitterTexture = (*textures.begin()); //TODO find a default emitter texture
}

void Emitter::Update(float dt)
{

	//---
}

void Emitter::CleanUp()
{
	for (auto mod = modules.begin(); mod != modules.end(); ++mod)
	{
		delete (*mod);
	}

	modules.clear();
	App->resourceManager->FreeResource(emitterTexture->GetUID());
}

void Emitter::Save(ParsonNode& node)
{
	node.SetString("name", name.c_str());

	uint32 textureUID = (emitterTexture != nullptr) ? emitterTexture->GetUID() : 0;
	node.SetInteger("textureUID", textureUID);
	node.SetString("texturePath", emitterTexture->GetAssetsPath());

	node.SetInteger("maxParticleCount", maxParticleCount);

	ParsonArray modulesArray = node.SetArray("modules");

	for (auto mod = modules.begin(); mod != modules.end(); ++mod)
	{
		(*mod)->Save(modulesArray.SetNode("module"));
	}
}

void Emitter::Load(ParsonNode& node)
{
	node.SetString("name", name.c_str());
	
	std::string path = node.GetString("texturePath");
	emitterTexture = (R_Texture*)App->resourceManager->GetResourceFromLibrary(path.c_str());

	maxParticleCount = node.GetInteger("maxParticleCount");

	ParsonArray modulesArray = node.GetArray("modules");

	for (uint i = 0 ; i < modulesArray.size ; ++i)
	{
		//(*mod)->Save(modulesArray.SetNode("module"));
		ParsonNode mod = modulesArray.GetNode(i);

		ParticleModule* particleModule = nullptr;

		switch ((ParticleModule::Type)mod.GetInteger("Type"))
		{
			case ParticleModule::Type::EMITTER_BASE: particleModule = new EmitterBase(); break;
			case ParticleModule::Type::EMITTER_SPAWN: particleModule = new EmitterSpawn(); break;
			case ParticleModule::Type::PARTICLE_LIFETIME: particleModule = new ParticleLifetime(); break;
			case ParticleModule::Type::PARTICLE_COLOR: particleModule = new ParticleColor(); break;
			case ParticleModule::Type::PARTICLE_MOVEMENT: particleModule = new ParticleMovement(); break;
		}

		particleModule->Load(mod);

		modules.push_back(particleModule);
	}
}

void Emitter::SetAsDefault()
{
	AddModuleFromType(ParticleModule::Type::EMITTER_BASE);
	AddModuleFromType(ParticleModule::Type::EMITTER_SPAWN);
	AddModuleFromType(ParticleModule::Type::PARTICLE_LIFETIME);
	AddModuleFromType(ParticleModule::Type::PARTICLE_COLOR);
	AddModuleFromType(ParticleModule::Type::PARTICLE_MOVEMENT);
	AddModuleFromType(ParticleModule::Type::PARTICLE_BILLBOARDING);
}

bool Emitter::AddModuleFromType(ParticleModule::Type type)
{
	//switch (type) and in each case a type of particlemodule to add to the emitter.
	for (int i = 0; i < modules.size(); i++)
	{
		if (modules[i]->type == type)
		{
			return false;
		}
	}

	switch (type)
	{
		case (ParticleModule::Type::EMITTER_BASE):
			modules.push_back(new EmitterBase); 
			break;
		case (ParticleModule::Type::EMITTER_SPAWN):
			modules.push_back(new EmitterSpawn);
			break;
		//case(ParticleModule::Type::ParticlePosition):
		//	modules.push_back(new ParticlePosition);
		//	break;
		case(ParticleModule::Type::PARTICLE_COLOR):
			modules.push_back(new ParticleColor);
			break;	
		case(ParticleModule::Type::PARTICLE_LIFETIME):
			modules.push_back(new ParticleLifetime);
			break;
		case(ParticleModule::Type::PARTICLE_MOVEMENT):
			modules.push_back(new ParticleMovement);
			break;
		case(ParticleModule::Type::PARTICLE_BILLBOARDING):
			modules.push_back(new ParticleBillboarding);
			break;
		case(ParticleModule::Type::NONE):
			return false;
	}
	return true;

}

bool Emitter::DeleteModuleFromType(ParticleModule::Type type)
{
	//switch (type) and in each case a type of particlemodule to add to the emitter.
	for (int i = 0; i < modules.size(); i++)
	{
		if (modules[i]->type == type)
		{
			modules.erase(modules.begin() + i);
			modules.shrink_to_fit();
		}
	}
	return true;
}

void Emitter::SetTexture(R_Texture* newTexture)
{
	if (newTexture != nullptr)
	{
		R_Texture* a = (R_Texture*)App->resourceManager->GetResourceFromLibrary(newTexture->GetAssetsPath());

		if (a != nullptr)
		{
			App->resourceManager->FreeResource(emitterTexture->GetUID());
			emitterTexture = a;
		}
		else
		{
			LOG("COuld not find Texture %s for emitter", newTexture->GetAssetsFile());
		}

	}
}
