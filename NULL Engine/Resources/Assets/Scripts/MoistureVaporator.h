#pragma once

#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;

enum class VaporatorState
{
	IDLE,		// Waiting to be activated
	STUNNING,	// Stunn activation
	REFRESHING	// On cooldown
};

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

	float stunDuration = 1.f;

	// Audio

	C_AudioSource* moistureAudio = nullptr;

	float cooldown = 1.0f;
	float particleEmitttingTime = 1.0f;

private:
	GameObject* gameManager = nullptr;
	GameObject* vaporatorObject = nullptr;
	C_BoxCollider* vaporatorCollider = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;
	C_ParticleSystem* idleParticles = nullptr;

	VaporatorState state;

	Timer cooldownTimer;
	Timer explosionParticlesTimer;

};

SCRIPTS_FUNCTION MoistureVaporator* CreateMoistureVaporator() {
	MoistureVaporator* script = new MoistureVaporator();
	INSPECTOR_DRAGABLE_FLOAT(script->stunDuration);
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->vaporatorObjectName);
	INSPECTOR_INPUT_FLOAT3(script->vaporatorColliderSize);
	INSPECTOR_INPUT_FLOAT3(script->stunTriggerSize);
	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->cooldown);
	INSPECTOR_DRAGABLE_FLOAT(script->particleEmitttingTime);
	return script;
}
