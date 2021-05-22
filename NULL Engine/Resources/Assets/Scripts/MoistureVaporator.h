#pragma once

#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;

class SCRIPTS_API MoistureVaporator : public Object ALLOWED_INHERITANCE {
public:
	MoistureVaporator();
	~MoistureVaporator();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void OnTriggerRepeat(GameObject* object) override;

	std::string gameManagerName;
	std::string vaporatorObjectName;

	float3 vaporatorColliderSize = float3::zero;
	float3 stunTriggerSize = float3::zero;

	int damage = 0;

	float power = 2000000.0f;
	float particleEmitttingTime = 1.f;

	float stunDuration = 1.f;

	// Audio

	C_AudioSource* explosion = nullptr;

private:
	GameObject* gameManager = nullptr;
	GameObject* vaporatorObject = nullptr;
	C_BoxCollider* vaporatorCollider = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;

	float particleTimer = 0.f;

	bool toExplode = false;
	bool exploded = false;
};

SCRIPTS_FUNCTION MoistureVaporator* CreateMoistureVaporator() {
	MoistureVaporator* script = new MoistureVaporator();
	INSPECTOR_DRAGABLE_FLOAT(script->stunDuration);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->vaporatorObjectName);
	INSPECTOR_INPUT_FLOAT3(script->vaporatorColliderSize);
	INSPECTOR_INPUT_FLOAT3(script->stunTriggerSize);
	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->particleEmitttingTime);
	return script;
}
