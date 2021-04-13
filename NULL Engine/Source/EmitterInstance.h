#ifndef __EMITTER_INSTANCE_H__
#define __EMITTER_INSTANCE_H__

#include "Globals.h"
#include "Particle.h"

#include <vector>

class C_ParticleSystem;
class Emitter;

class EmitterInstance
{
public:
	EmitterInstance();
	~EmitterInstance();

	//if emitter equals null, a default emitter is added
	void Init(Emitter* emitter, C_ParticleSystem* component);		
	void Update(float dt);
	void DrawParticles();

	void SpawnParticle();
	void ResetEmitter();

private:	
public:

	void UpdateModules(float dt);
	void KillDeadParticles();
	void KillAll();

public:
	unsigned int activeParticles = 0;
	std::vector<Particle> particles;
	unsigned int* particleIndices;
	
	float emitterTime = 0.0f;

	Emitter* emitter;
	C_ParticleSystem* component;
};

#endif //!__EMITTER_INSTANCE_H__