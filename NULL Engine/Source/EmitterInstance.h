#ifndef __EMITTER_INSTANCE_H__
#define __EMITTER_INSTANCE_H__

#include "Globals.h"
#include "Particle.h"
#include "Emitter.h"
//#include "C_ParticleSystem"

#include <vector>

class EmitterInstance
{
public:
	EmitterInstance();
	~EmitterInstance();

public:
	std::vector<Particle> particles;
	Emitter emitter;
	//C_ParticleSystem* owner;

};


#endif //!__EMITTER_INSTANCE_H__