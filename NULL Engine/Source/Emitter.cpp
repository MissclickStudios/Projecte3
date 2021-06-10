#include "JSONParser.h"
#include "Application.h"

#include "ResourceBase.h"
#include "M_ResourceManager.h"

#include "ParticleModule.h"

#include "Resource.h"
#include "R_Texture.h"

#include "Emitter.h"

#include "MemoryManager.h"

Emitter::Emitter()
{
	std::vector<ResourceBase> textures;
	App->resourceManager->GetResourceBases<R_Texture>(textures);
	emitterTexture = (R_Texture*)App->resourceManager->GetResourceFromLibrary(textures.begin()->assetsPath.c_str());

	textures.clear();
}

Emitter::Emitter(const char* name)
{
	std::vector<ResourceBase> textures;
	App->resourceManager->GetResourceBases<R_Texture>(textures);
	emitterTexture = (R_Texture*)App->resourceManager->GetResourceFromLibrary(textures.begin()->assetsPath.c_str());
	this->name = name;
}

void Emitter::Update(float dt)
{

	//---
}

void Emitter::CleanUp()
{
	LOG("EMITTER CLEANUP");

	for (auto mod = modules.begin(); mod != modules.end(); ++mod)
	{
		RELEASE((*mod));
	}

	modules.clear();
	
	if (emitterTexture != nullptr) 
	{
		App->resourceManager->FreeResource(emitterTexture->GetUID());
		emitterTexture = nullptr;
	}
}

void Emitter::Save(ParsonNode& node)
{
	node.SetString("name", name.c_str());

	uint32 textureUID = (emitterTexture != nullptr) ? emitterTexture->GetUID() : 0;
	node.SetInteger("textureUID", textureUID);
	
	if(emitterTexture != nullptr)
		node.SetString("texturePath", emitterTexture->GetAssetsPath());
	else
		node.SetString("texturePath", path.c_str());
	

	node.SetInteger("maxParticleCount", maxParticleCount);

	ParsonArray modulesArray = node.SetArray("modules");

	for (auto mod = modules.begin(); mod != modules.end(); ++mod)
	{
		(*mod)->Save(modulesArray.SetNode("module"));
	}
}

void Emitter::Load(ParsonNode& node)
{
	name = node.GetString("name");
	
	path = node.GetString("texturePath");
	emitterTexture = (R_Texture*)App->resourceManager->GetResourceFromLibrary(path.c_str());

	maxParticleCount = node.GetInteger("maxParticleCount");

	ParsonArray modulesArray = node.GetArray("modules");

	for (uint i = 0 ; i < modulesArray.size ; ++i)
	{
		//(*mod)->Save(modulesArray.SetNode("module"));
		ParsonNode mod = modulesArray.GetNode(i);

		ParticleModule* particleModule = nullptr;

		ParticleModule::Type a = (ParticleModule::Type)mod.GetInteger("Type");

		switch (a)
		{
			case ParticleModule::Type::EMITTER_BASE: particleModule = new EmitterBase(); break;
			case ParticleModule::Type::EMITTER_SPAWN: particleModule = new EmitterSpawn(); break;
			case ParticleModule::Type::PARTICLE_LIFETIME: particleModule = new ParticleLifetime(); break;
			case ParticleModule::Type::PARTICLE_COLOR: particleModule = new ParticleColor(); break;
			case ParticleModule::Type::PARTICLE_MOVEMENT: particleModule = new ParticleMovement(); break;
			case ParticleModule::Type::EMITTER_AREA: particleModule = new EmitterArea(); break;
			//case ParticleModule::Type::PARTICLE_ROTATION: particleModule = new ParticleRotation(); break;
			case ParticleModule::Type::PARTICLE_SIZE: particleModule = new ParticleSize(); break;
			case ParticleModule::Type::PARTICLE_BILLBOARDING: particleModule = new ParticleBillboarding(); break;
		}

		if (particleModule != nullptr)
		{
			particleModule->Load(mod);
			modules.push_back(particleModule);
		}
		else
		{
			LOG("Could not load particle module with type: %d", (int)a);
		}
	}
}

void Emitter::SetAsDefault()
{
	AddModuleFromType(ParticleModule::Type::EMITTER_BASE);
	AddModuleFromType(ParticleModule::Type::EMITTER_SPAWN);
	AddModuleFromType(ParticleModule::Type::EMITTER_AREA);
	AddModuleFromType(ParticleModule::Type::PARTICLE_LIFETIME);
	AddModuleFromType(ParticleModule::Type::PARTICLE_COLOR);
	AddModuleFromType(ParticleModule::Type::PARTICLE_MOVEMENT);
	AddModuleFromType(ParticleModule::Type::PARTICLE_SIZE);
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
		case (ParticleModule::Type::EMITTER_AREA):
			modules.push_back(new EmitterArea);
			break;
		case(ParticleModule::Type::PARTICLE_COLOR):
			modules.push_back(new ParticleColor);
			break;	
		case(ParticleModule::Type::PARTICLE_LIFETIME):
			modules.push_back(new ParticleLifetime);
			break;
		case(ParticleModule::Type::PARTICLE_MOVEMENT):
			modules.push_back(new ParticleMovement);
			break;
		case(ParticleModule::Type::PARTICLE_SIZE):
			modules.push_back(new ParticleSize);
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

void Emitter::SetParticleCount(int particleCount)
{
	maxParticleCount = particleCount;
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

void Emitter::SetTexture(ResourceBase newTexture)
{
	
	R_Texture* a = (R_Texture*)App->resourceManager->GetResourceFromLibrary(newTexture.assetsPath.c_str());

	if (a != nullptr)
	{
		if(emitterTexture !=nullptr)
			App->resourceManager->FreeResource(emitterTexture->GetUID());

		emitterTexture = a;
	}
	else
	{
		LOG("COuld not find Texture %s for emitter", newTexture.assetsPath.c_str());
	}

	
}
