#ifndef __EMITTER_INSTANCE_H__
#define __EMITTER_INSTANCE_H__

#include "Globals.h"
#include "Particle.h"
#include "Emitter.h"

#include <vector>

class C_ParticleSystem;

class EmitterInstance
{
public:
	EmitterInstance();
	~EmitterInstance();

	void Init();
	void UpdateModules();
	void DrawParticles();

private:
	void KillDeadParticles();
	void KillAll();

public:
	std::vector<Particle> particles;
	
	Emitter* emitter;
	C_ParticleSystem* component;
};


#endif //!__EMITTER_INSTANCE_H__