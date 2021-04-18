#ifndef __C_PARTICLESYSTEM_H__
#define __C_PARTICLESYSTEM_H__

#include "Component.h"
#include "EmitterInstance.h"

class R_ParticleSystem;
class ResourceBase;

class NULL_API C_ParticleSystem : public Component
{
public:
	C_ParticleSystem(GameObject* owner);
	~C_ParticleSystem();

	bool Update() override;
	bool CleanUp() override;
	
	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

public:
	void Reset();

	void SetParticleSystem(R_ParticleSystem* newParticleSystem);
	void SetParticleSystem(ResourceBase newParticleSystem);

	void RefreshEmitterInstances();

	void AddParticleSystem(const char* name);
	void SaveParticleSystem() const;

	bool SetAsDefaultComponent();					//Reset the component, add an emitterInstance to the list and assign it the default emitter
	void AddDefaultEmitter();						//Create a default emitter

	void EnginePreview(bool previewEnabled);

	void StopSpawn();
	void ResumeSpawn();

	void StopAndDelete();

private:
	void ClearEmitters();
	void InternalStopAndDelete();

public:
	std::vector<EmitterInstance*> emitterInstances; // don't save/load
	
	R_ParticleSystem* resource = nullptr;			// save/load

	bool stopSpawn = false;
	bool previewEnabled = false;
	bool tempDelete = false;

private:
	bool stopAndDeleteCheck = false;

};

#endif //!__C_PARTICLES_H__
