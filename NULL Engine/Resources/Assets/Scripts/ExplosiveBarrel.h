#include "Object.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

#include "Timer.h"

#include <string>

class GameObject;
class C_BoxCollider;
class C_AudioSource;
class C_ParticleSystem;

class SCRIPTS_API ExplosiveBarrel : public Object ALLOWED_INHERITANCE 
{
public:

	ExplosiveBarrel();
	~ExplosiveBarrel();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void OnPause() override;
	void OnResume() override;

	float particleEmitttingTime = 1.0f;

	std::string explosionName;
	std::string markerName;
	std::string particleName;

	bool reload = false;
	float cooldown = 5.0f;

	// Audio
	C_AudioSource* explosionAudio = nullptr;

	bool toExplode = false;
	bool exploded = false;
	bool shit = false;

private:

	Timer particleTimer;
	Timer cooldownTimer;

	C_BoxCollider* explosionCollider = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;
	GameObject* explosionMarker = nullptr;
	C_ParticleSystem* activeParticles = nullptr;
};

SCRIPTS_FUNCTION ExplosiveBarrel* CreateExplosiveBarrel();
