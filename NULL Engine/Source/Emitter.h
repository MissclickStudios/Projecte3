#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <vector>

#include "M_ParticleSystem.h"
#include "Particle.h"

class R_Material;

class NULL_API Emitter
{
public:
	Emitter();

	//Loop through all modules, loop through particles, update them     
	void Update(float dt);

	//Save and load?

	void SetAsDefault();
	bool AddModuleFromType(M_ParticleSystem::Type type);
	bool DeleteModuleFromType(M_ParticleSystem::Type type);

	void SetMaterial();

public:
	std::string name = "Particle Emitter";
	std::vector<M_ParticleSystem*> modules;
	R_Material* emitterMaterial;

	int maxParticleCount = 10;
};

#endif // !__EMITTER_H__
