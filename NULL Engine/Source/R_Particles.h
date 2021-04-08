#ifndef __R_PARTICLE_SYSTEM_H__
#define __R_PARTICLE_SYSTEM_H__

#include "Resource.h"
#include "Emitter.h"

class R_Particles : public Resource
{
public:
	R_Particles();
	~R_Particles();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:
	void AddDefaultEmitter();																			//default emitter creation

public:
	std::vector<Emitter> emitters;
};

#endif //!__R_PARTICLE_SYSTEM_H__
