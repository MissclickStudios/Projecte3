#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_Animator;

enum class SarlaacState
{
	IDLE,	//Waiting to be activated
	MOVING, //Animating towards DAMAGING
	DAMAGING, //Damage for 1 frame
	SLEEPING  //Waits for a brief time until going back to IDLE
};

class SCRIPTS_API SarlaacTrap : public Script {
public:
	SarlaacTrap();
	~SarlaacTrap();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnTriggerRepeat(GameObject* object) override;

	void StartMoving();

	std::string animationName = "Eat";

	int damage = 0;

	float activationTime = 1.f;
	float sleepingTime = 2.f;

private:

	float animationTimer = 0.0f;

	C_Animator* sarlaacAnimator = nullptr;

	SarlaacState state = SarlaacState::IDLE;
};

SCRIPTS_FUNCTION SarlaacTrap* CreateSarlaacTrap() {
	SarlaacTrap* script = new SarlaacTrap();
	INSPECTOR_INPUT_INT(script->damage);
	INSPECTOR_STRING(script->animationName);
	INSPECTOR_DRAGABLE_FLOAT(script->activationTime);
	INSPECTOR_DRAGABLE_FLOAT(script->sleepingTime);
	return script;
}