#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"

#define BULLET_AMOUNT_SAND_TROOPER 5

enum class SandTrooperState;
struct Projectile;

//struct ProjectileSandTrooper
//{
//	ProjectileSandTrooper() : inUse(false), object(nullptr) {}
//	ProjectileSandTrooper(GameObject* object) : inUse(false), object(object) {}
//	ProjectileSandTrooper(bool inUse, GameObject* object) : inUse(inUse), object(object) {}
//
//	bool inUse;
//	GameObject* object;
//};

class SCRIPTS_API SandTrooper : public Script {
public:
	SandTrooper();
	~SandTrooper();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	void OnCollisionEnter(GameObject* object) override;

	Projectile* CreateProjectile(uint index);

private:

	bool dead = false;

	Prefab bullet;

	SandTrooperState state;

	C_Transform* playerTransform = nullptr;

	//Bullets
	GameObject* bulletStorage = nullptr;
	Projectile* bullets[BULLET_AMOUNT_SAND_TROOPER];
	
};

SCRIPTS_FUNCTION SandTrooper* CreateSandTrooper() {
	SandTrooper* script = new SandTrooper();
	return script;
}