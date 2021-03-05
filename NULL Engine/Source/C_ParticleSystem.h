#ifndef __C_PARTICLE_SYSTEM_H__
#define __C_PARTICLE_SYSTEM_H__

#include "Component.h"
#include "EmitterInstance.h"

class C_ParticleSystem : public Component
{
public:
	C_ParticleSystem(GameObject* owner);
	~C_ParticleSystem();

	//save/load?

	bool Update(float dt);
	void Reset();

	//set resource?
	
public:
	std::vector<EmitterInstance> emitterInstances;
};

#endif //!__C_PARTICLE_SYSTEM_H__
