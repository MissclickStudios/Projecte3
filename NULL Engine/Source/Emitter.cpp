#include "Emitter.h"

#include "MemoryManager.h"

Emitter::Emitter()
{
}

void Emitter::Update(float dt)
{

	//---
}

void Emitter::SetAsDefault()
{
	AddModuleFromType(M_ParticleSystem::EmitterBase);
	AddModuleFromType(M_ParticleSystem::EmitterSpawn);
	AddModuleFromType(M_ParticleSystem::ParticleLifetime);
	AddModuleFromType(M_ParticleSystem::ParticleColor);
	AddModuleFromType(M_ParticleSystem::ParticleMovement);
}

bool Emitter::AddModuleFromType(M_ParticleSystem::Type type)
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
		case (M_ParticleSystem::Type::EmitterBase):
			modules.push_back(new EmitterBase); 
			break;
		case (M_ParticleSystem::Type::EmitterSpawn):
			modules.push_back(new EmitterSpawn);
			break;
		//case(ParticleModule::Type::ParticlePosition):
		//	modules.push_back(new ParticlePosition);
		//	break;
		case(M_ParticleSystem::Type::ParticleColor):
			modules.push_back(new ParticleColor);
			break;	
		case(M_ParticleSystem::Type::ParticleLifetime):
			modules.push_back(new ParticleLifetime);
			break;
		case(M_ParticleSystem::Type::ParticleMovement):
			modules.push_back(new ParticleMovement);
			break;
		case(M_ParticleSystem::Type::None):
			return false;
	}
	return true;

}

bool Emitter::DeleteModuleFromType(M_ParticleSystem::Type type)
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
