#pragma once
#include "Object.h"
#include "ScriptMacros.h"

#include "Effect.h"

#include <vector>

class C_RigidBody;
class Weapon;

class SCRIPTS_API Bullet : public Object ALLOWED_INHERITANCE
{
public:

	Bullet();
	virtual ~Bullet();

	void Awake() override;
	void Update() override;
	void CleanUp() override;

	void OnPause() override;
	void OnResume() override;

	void OnCollisionEnter(GameObject* object) override;

	void SetShooter(Weapon* shooter, int index);
	void SetOnHitData(float damage, std::vector<Effect> effects, float lifeTime);

private:

	C_RigidBody* rigidBody = nullptr;

	Weapon* shooter = nullptr;
	int index = 0;
	float onHitdamage = 0.0f;
	std::vector<Effect> onHitEffects;

	float lifeTime = 0.0f;
	Timer lifeTimeTimer;

	bool hit = false;
};

SCRIPTS_FUNCTION Bullet* CreateBullet();