
#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

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

	float particleEmitttingTime = 1.f;

	std::string explosionName;

	// Audio
	C_AudioSource* explosion = nullptr;
	std::string explosionAudio = "item_barrel_explosion";

	bool toExplode = false;
	bool exploded = false;
	bool shit = false;

private:

	GameObject* explosionGameObject = nullptr;
	C_ParticleSystem* explosionParticles = nullptr;
	
	float particleTimer = 0.f;
};

SCRIPTS_FUNCTION ExplosiveBarrel* CreateExplosiveBarrel();
