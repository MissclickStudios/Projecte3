#ifndef __C_PARTICLESYSTEM_H__
#define __C_PARTICLESYSTEM_H__

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

	void SetParticleSystem(R_ParticleSystem* newParticleSystem);
	void RefreshEmitters();

	void NewParticleSystem();
	void SaveParticleSystem();

	bool SetAsDefaultComponent();		//Reset the component, add an emitterInstance to the list and assign it the default emitter
	void AddDefaultEmitter();			//Create a default emitter

	void EnginePreview(bool previewEnabled);

	void StopSpawn();
	void ResumeSpawn();

	void StopAndDelete();

private:
	void ClearEmitters();
	void InternalStopAndDelete();

public:
	Emitter* defaultEmitter;
	std::vector<EmitterInstance*> emitterInstances;
	R_ParticleSystem* resource = nullptr;

	bool stopSpawn = false;
	bool previewEnabled = false;
	bool tempDelete = false;

private:
	bool stopAndDeleteCheck = false;
};

#endif //!__C_PARTICLES_H__
