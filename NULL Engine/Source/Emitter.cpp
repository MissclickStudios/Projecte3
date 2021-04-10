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
	AddModuleFromType(ParticleModule::EmitterBase);
	AddModuleFromType(ParticleModule::EmitterSpawn);
	AddModuleFromType(ParticleModule::ParticleLifetime);
	AddModuleFromType(ParticleModule::ParticleColor);
	AddModuleFromType(ParticleModule::ParticleMovement);
	AddModuleFromType(ParticleModule::ParticleBillboarding);
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
