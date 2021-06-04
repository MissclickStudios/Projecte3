
#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;
class C_ParticleSystem;

class SCRIPTS_API ExplosiveBarrel : public Object ALLOWED_INHERITANCE {
public:
	ExplosiveBarrel();
	~ExplosiveBarrel();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	float particleEmitttingTime = 1.f;

	float3 barrelColliderSize = float3::zero;
	float3 explosionTriggerSize = float3::zero;

	// Audio
	C_AudioSource* explosion = nullptr;

	bool toExplode = false;
	bool exploded = false;
	bool shit = false;

private:

	C_BoxCollider* barrelCollider = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;
	
	float particleTimer = 0.f;
};

SCRIPTS_FUNCTION ExplosiveBarrel* CreateExplosiveBarrel();
