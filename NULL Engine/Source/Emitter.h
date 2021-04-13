#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <vector>

#include "ParticleModule.h"

class R_Texture;
class ParsonNode;

class NULL_API Emitter
{
public:
	Emitter();

	//Loop through all modules, loop through particles, update them     
	void Update(float dt);

	//Save and load?
	void Save(ParsonNode& node);
	void Load(ParsonNode& node);

	void SetAsDefault();
	bool AddModuleFromType(ParticleModule::Type type);
	bool DeleteModuleFromType(ParticleModule::Type type);

	void SetMaterial();

public:
	std::string name = "Particle Emitter";
	std::vector<ParticleModule*> modules;
	R_Texture* emitterTexture;

	int maxParticleCount = 10;
};

#endif // !__EMITTER_H__
