#pragma once
#include "Script.h"

class GameObject;

enum class ObjectType
{
	NONE,
	ENTITY,
	BULLET,
	COLLECTABLE
};

class CollideableObject : public Script
{
public:
	CollideableObject() {}
	virtual ~CollideableObject() {}

	virtual void Awake() override {}
	virtual void Start() override {}

	virtual void PreUpdate() override {}
	virtual void Update() override {}
	virtual void PostUpdate() override {}

	virtual void CleanUp() override {}

	virtual void OnDisable() override {}
	virtual void OnEnable() override {}

	virtual void OnCollisionEnter(GameObject* object) override {}
	virtual void OnCollisionRepeat(GameObject* object) override {}
	virtual void OnCollisionExit(GameObject* object) override {}
	virtual void OnTriggerEnter(GameObject* object) override {}
	virtual void OnTriggerRepeat(GameObject* object) override {}
	virtual void OnTriggerExit(GameObject* object) override {}

	ObjectType baseType = ObjectType::NONE;
};

CollideableObject* GetCollideableScript(GameObject* gameObject, ObjectType baseType);