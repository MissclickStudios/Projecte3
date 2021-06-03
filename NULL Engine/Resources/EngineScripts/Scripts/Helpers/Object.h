#pragma once
#include "Script.h"

class GameObject;

enum class ObjectType
{
	NONE,
	ENTITY,
	WEAPON,
	BULLET,
	COLLECTABLE,
	EXPLOSIVE_BARREL,
	GROUND_ITEM,
	WEAPON_ITEM,
	GROGU_ABILITY
};

class Object : public Script
{
public:

	Object() {}
	virtual ~Object() {}

	virtual void Awake() override {}
	virtual void Start() override {}

	virtual void PreUpdate() override {}
	virtual void Update() override {}
	virtual void PostUpdate() override {}

	virtual void CleanUp() override {}

	void Activate();
	void Deactivate();

	virtual void OnDisable() override {}
	virtual void OnEnable() override {}

	virtual void OnPause() override { paused = true; }
	virtual void OnResume() override { paused = false; }

	virtual void OnCollisionEnter(GameObject* object) override {}
	virtual void OnCollisionRepeat(GameObject* object) override {}
	virtual void OnCollisionExit(GameObject* object) override {}
	virtual void OnTriggerEnter(GameObject* object) override {}
	virtual void OnTriggerRepeat(GameObject* object) override {}
	virtual void OnTriggerExit(GameObject* object) override {}

	ObjectType baseType = ObjectType::NONE;

protected:

	bool paused = false;
};

Object* GetObjectScript(GameObject* gameObject, ObjectType baseType = ObjectType::NONE);