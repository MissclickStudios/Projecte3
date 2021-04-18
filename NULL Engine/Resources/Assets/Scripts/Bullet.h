#pragma once
#include "CollideableObject.h"
#include "ScriptMacros.h"

#include "Effect.h"

#include <vector>

class Weapon;

class SCRIPTS_API Bullet : public CollideableObject ALLOWED_INHERITANCE
{
public:

	Bullet();
	virtual ~Bullet();

	void Update() override;

	void OnCollisionEnter(GameObject* object) override;

	void SetShooter(Weapon* shooter, int index);
	void SetOnHitData(float damage, std::vector<Effect> effects);

private:

	Weapon* shooter = nullptr;
	int index = 0;
	float onHitdamage = 0.0f;
	std::vector<Effect> onHitEffects;

	bool hit = false;
};

SCRIPTS_FUNCTION Bullet* CreateBullet();