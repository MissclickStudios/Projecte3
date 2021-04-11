#include "JSONParser.h"
#include "Application.h"
#include "M_ResourceManager.h"

#include "ParticleModule.h"

#include "R_Texture.h"

#include "Emitter.h"

#include "MemoryManager.h"

Emitter::Emitter()
{
}

void Emitter::Update(float dt)
{

	//---
}

void Emitter::Save(ParsonNode& node)
{
	node.SetString("name", name.c_str());
	node.SetInteger("textureUID", emitterTexture->GetUID());
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
	
	uint textureUID = node.GetInteger("textureUID");
	if (App->resourceManager->AllocateResource(textureUID))
	{
		emitterTexture = (R_Texture*)App->resourceManager->RequestResource(textureUID); //TODO PARTICLE SYSTEM
	}

	maxParticleCount = node.GetInteger("maxParticleCount");

	ParsonArray modulesArray = node.GetArray("modules");

	for (uint i = 0 ; i < modulesArray.size ; ++i)
	{
		//(*mod)->Save(modulesArray.SetNode("module"));
		ParsonNode mod = modulesArray.GetNode(i);

		ParticleModule* particleModule = nullptr;

		switch ((ParticleModule::Type)mod.GetInteger("Type"))
		{
			case ParticleModule::Type::EmitterBase: particleModule = new EmitterBase(); break;
			case ParticleModule::Type::EmitterSpawn: particleModule = new EmitterSpawn(); break;
			case ParticleModule::Type::ParticleLifetime: particleModule = new ParticleLifetime(); break;
			case ParticleModule::Type::ParticleColor: particleModule = new ParticleColor(); break;
			case ParticleModule::Type::ParticleMovement: particleModule = new ParticleMovement(); break;
		}

		particleModule->Load(mod);

		modules.push_back(particleModule);
	}
}

void Emitter::SetAsDefault()
{
	AddModuleFromType(ParticleModule::Type::EmitterBase);
	AddModuleFromType(ParticleModule::Type::EmitterSpawn);
	AddModuleFromType(ParticleModule::Type::ParticleLifetime);
	AddModuleFromType(ParticleModule::Type::ParticleColor);
	AddModuleFromType(ParticleModule::Type::ParticleMovement);
	AddModuleFromType(ParticleModule::Type::ParticleBillboarding);
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
		case (ParticleModule::Type::EmitterBase):
			modules.push_back(new EmitterBase); 
			break;
		case (ParticleModule::Type::EmitterSpawn):
			modules.push_back(new EmitterSpawn);
			break;
		//case(ParticleModule::Type::ParticlePosition):
		//	modules.push_back(new ParticlePosition);
		//	break;
		case(ParticleModule::Type::ParticleColor):
			modules.push_back(new ParticleColor);
			break;	
		case(ParticleModule::Type::ParticleLifetime):
			modules.push_back(new ParticleLifetime);
			break;
		case(ParticleModule::Type::ParticleMovement):
			modules.push_back(new ParticleMovement);
			break;
		case(ParticleModule::Type::ParticleBillboarding):
			modules.push_back(new ParticleBillboarding);
			break;
		case(ParticleModule::Type::None):
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
