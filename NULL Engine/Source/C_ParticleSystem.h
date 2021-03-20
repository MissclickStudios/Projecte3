#ifndef __C_PARTICLE_SYSTEM_H__
#define __C_PARTICLE_SYSTEM_H__

#include "Component.h"
#include "EmitterInstance.h"

class NULL_API C_ParticleSystem : public Component
{
public:
	C_ParticleSystem(GameObject* owner);
	~C_ParticleSystem();

	//save/load?

	bool Update();
	void Reset();

	bool SetAsDefaultComponent();		//Reset the component, add an emitterInstance to the list and assign it the default emitter
	void AddDefaultEmitter();			//Create a default emitter

public:
	Emitter* defaultEmitter;
	std::vector<EmitterInstance*> emitterInstances;
};

#endif //!__C_PARTICLE_SYSTEM_H__
