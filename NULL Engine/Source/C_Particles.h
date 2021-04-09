#ifndef __C_PARTICLES_H__
#define __C_PARTICLES_H__

#include "Component.h"
#include "EmitterInstance.h"

class R_Particles;

class NULL_API C_Particles : public Component
{
public:
	C_Particles(GameObject* owner);
	~C_Particles();

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
	R_Particles* resource = nullptr;

	bool previewEnabled = false;
};

#endif //!__C_PARTICLES_H__
