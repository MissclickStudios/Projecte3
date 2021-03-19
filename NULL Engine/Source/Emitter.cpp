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
}

bool Emitter::AddModuleFromType(ParticleModule::Type type)
{
	//switch (type) and in each case a type of particlemodule to add to the emitter.
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
		case(ParticleModule::Type::None):
			return false;
	}
	return true;

}
