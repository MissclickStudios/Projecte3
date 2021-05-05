#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;

class SCRIPTS_API SarlaacTrap : public Script {
public:
	SarlaacTrap();
	~SarlaacTrap();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	void OnTriggerRepeat(GameObject* object) override;

	std::string gameManagerName;
	std::string explosionObjectName;

	float3 barrelColliderSize = float3::zero;
	float3 explosionTriggerSize = float3::zero;

	int damage = 0;

private:
	GameObject* gameManager = nullptr;
	GameObject* explosionObject = nullptr;
	C_BoxCollider* barrelCollider = nullptr;

	bool toExplode = false;
	bool exploded = false;
};

SCRIPTS_FUNCTION SarlaacTrap* CreateSarlaacTrap() {
	SarlaacTrap* script = new SarlaacTrap();
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->explosionObjectName);
	INSPECTOR_INPUT_FLOAT3(script->barrelColliderSize);
	INSPECTOR_INPUT_FLOAT3(script->explosionTriggerSize);
	INSPECTOR_INPUT_INT(script->damage);
	return script;
}