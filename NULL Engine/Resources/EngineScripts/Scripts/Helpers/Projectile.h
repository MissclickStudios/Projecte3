#pragma once
class GameObject;

struct Projectile
{
	Projectile() : inUse(false), object(nullptr) {}
	Projectile(GameObject* object) : inUse(false), object(object) {}
	Projectile(bool inUse, GameObject* object) : inUse(inUse), object(object) {}

	bool inUse;
	GameObject* object;
};
