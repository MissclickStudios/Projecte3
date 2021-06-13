#include "Object.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_Animator;
class C_ParticleSystem;
class C_AudioSource;

enum class SarlaacState
{
	IDLE,	//Waiting to be activated
	MOVING, //Animating towards DAMAGING
	DAMAGING, //Damage for 1 frame
	SLEEPING  //Waits for a brief time until going back to IDLE
};

class SCRIPTS_API SarlaacTrap : public Object ALLOWED_INHERITANCE{
public:
	SarlaacTrap();
	~SarlaacTrap();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnTriggerRepeat(GameObject* object) override;

	void StartMoving();

	std::string animationName = "Eat";

	int mandoDamage = 0.5;
	int enemyDamage = 20;

	float activationTime = 1.f;
	float sleepingTime = 2.f;

	bool initialized = false;

private:

	float animationTimer = 0.0f;

	C_AudioSource* sarlaacAudio = nullptr;
	C_AudioSource* sarlaccAttackAudio = nullptr;
	C_Animator* sarlaacAnimator = nullptr;

	C_ParticleSystem* idleParticles		= nullptr;
	C_ParticleSystem* attackParticles	= nullptr;

	SarlaacState state = SarlaacState::IDLE;
};

SCRIPTS_FUNCTION SarlaacTrap* CreateSarlaacTrap();