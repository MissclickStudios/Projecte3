
#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;

class SCRIPTS_API ExplosiveBarrel : public Object ALLOWED_INHERITANCE {
public:
	ExplosiveBarrel();
	~ExplosiveBarrel();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void OnTriggerRepeat(GameObject* object) override;

	std::string gameManagerName;
	std::string barrelObjectName;

	float3 barrelColliderSize = float3::zero;
	float3 explosionTriggerSize = float3::zero;

	int damage = 0;

	float power = 2000000.0f;
	float particleEmitttingTime = 1.f;

	// Audio

	C_AudioSource* explosion = nullptr;

private:
	GameObject* gameManager = nullptr;
	GameObject* barrelObject = nullptr;
	C_BoxCollider* barrelCollider = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;
	
	float particleTimer = 0.f;
	
	bool toExplode = false;
	bool exploded = false;
};

SCRIPTS_FUNCTION ExplosiveBarrel* CreateExplosiveBarrel() {
	ExplosiveBarrel* script = new ExplosiveBarrel();
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->barrelObjectName);
	INSPECTOR_INPUT_FLOAT3(script->barrelColliderSize);
	INSPECTOR_INPUT_FLOAT3(script->explosionTriggerSize);
	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->particleEmitttingTime);
	return script;
}
