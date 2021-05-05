
#include "Script.h"
#include "ScriptMacros.h"

class GameObject;

class SCRIPTS_API ExplosiveBarrel : public Script {
public:
	ExplosiveBarrel();
	~ExplosiveBarrel();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnCollisionEnter(GameObject* object) override;

	std::string gameManagerName;
	std::string explosionObjectName;

private:
	GameObject* gameManager = nullptr;
	GameObject* explosionObject = nullptr;
	bool toExplode = false;
};

SCRIPTS_FUNCTION ExplosiveBarrel* CreateExplosiveBarrel() {
	ExplosiveBarrel* script = new ExplosiveBarrel();
	INSPECTOR_STRING(script->gameManagerName);
	INSPECTOR_STRING(script->explosionObjectName);
	return script;
}
