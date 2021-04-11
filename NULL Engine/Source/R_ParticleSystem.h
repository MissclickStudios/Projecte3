#ifndef __R_PARTICLE_SYSTEM_H__
#define __R_PARTICLE_SYSTEM_H__

#include "Resource.h"
#include "Emitter.h"

class R_ParticleSystem : public Resource
{
public:
	R_ParticleSystem();
	~R_ParticleSystem();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:
	void AddDefaultEmitter();																			//default emitter creation

public:
	std::vector<Emitter> emitters;
};

#endif //!__R_PARTICLE_SYSTEM_H__
