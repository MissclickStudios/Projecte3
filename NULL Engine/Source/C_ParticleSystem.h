#ifndef __C_PARTICLE_SYSTEM_H__
#define __C_PARTICLE_SYSTEM_H__

#include "Component.h"
#include "EmitterInstance.h"

class R_ParticleSystem;

class NULL_API C_ParticleSystem : public Component
{
public:
	C_ParticleSystem(GameObject* owner);
	~C_ParticleSystem();

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	bool Update();
	void Reset();

	bool SetAsDefaultComponent();		//Reset the component, add an emitterInstance to the list and assign it the default emitter
	void AddDefaultEmitter();			//Create a default emitter

	void EnginePreview(bool previewEnabled);
private:
	void ClearEmitters();

public:
	Emitter* defaultEmitter;
	std::vector<EmitterInstance*> emitterInstances;
	R_ParticleSystem* resource = nullptr;

	bool previewEnabled = false;
};

#endif //!__C_PARTICLE_SYSTEM_H__
