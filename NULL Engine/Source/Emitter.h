#ifndef __EMITTER_H__
#define __EMITTER_H__

#include <vector>
#include "Particle.h"
#include "ParticleModule.h"

class R_Material;

class Emitter
{
public:
	Emitter();

	void Update(float dt);

	//Save and load?

	bool AddModuleFromType(ParticleModule::Type type);

public:
	std::string name = "Particle Emitter";
	std::vector<ParticleModule*> modules;

	int maxParticleCount = 10;
};

#endif // !__EMITTER_H__
