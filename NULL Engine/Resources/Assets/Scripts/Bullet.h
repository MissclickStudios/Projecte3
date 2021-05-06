#pragma once
#include "Entity.h"
#include "ScriptMacros.h"

#include "Effect.h"

#include <vector>

class Weapon;

class SCRIPTS_API Bullet : public Entity ALLOWED_INHERITANCE
{
public:

	Bullet();
	virtual ~Bullet();

	void SetUp() {}
	void Behavior() override;
	void CleanUp() {}

	void EntityPause() override;
	void EntityResume() override;

	void OnCollisionEnter(GameObject* object) override;

	void SetShooter(Weapon* shooter, int index);
	void SetOnHitData(float damage, std::vector<Effect> effects, float lifeTime);

private:

	Weapon* shooter = nullptr;
	int index = 0;
	float onHitdamage = 0.0f;
	std::vector<Effect> onHitEffects;

	float lifeTime = 0.0f;
	Timer lifeTimeTimer;

	bool hit = false;
};

SCRIPTS_FUNCTION Bullet* CreateBullet();