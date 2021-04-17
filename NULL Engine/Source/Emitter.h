#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <vector>

#include "ParticleModule.h"

class R_Texture;
class ParsonNode;
class ResourceBase;

class NULL_API Emitter
{
public:
	Emitter();

	//Loop through all modules, loop through particles, update them     
	void Update(float dt);
	void CleanUp();

	//Save and load?
	void Save(ParsonNode& node);
	void Load(ParsonNode& node);

	void SetAsDefault();
	bool AddModuleFromType(ParticleModule::Type type);
	bool DeleteModuleFromType(ParticleModule::Type type);

	void SetParticleCount(int particleCount);

	void SetTexture(R_Texture* newTexture);
	void SetTexture(ResourceBase newTexture);
public:
	std::vector<ParticleModule*> modules;
	
	std::string name = "Particle Emitter";
	R_Texture* emitterTexture = nullptr;

	int maxParticleCount = 10;
};

#endif // !__EMITTER_H__
