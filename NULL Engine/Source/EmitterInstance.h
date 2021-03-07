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

	void Init(Emitter* emitter, C_ParticleSystem* component);
	void Update(float dt);
	void DrawParticles();

	void Spawn();

private:	
	void UpdateModules();
	void KillDeadParticles();
	void KillAll();

public:
	unsigned int activeParticles = 0;
	std::vector<Particle> particles;
	
	Emitter* emitter;
	C_ParticleSystem* component;
};

#endif //!__EMITTER_INSTANCE_H__