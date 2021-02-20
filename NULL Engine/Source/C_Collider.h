#ifndef __C_COLLIDERCOMPONENT_H__
#define __C_COLLIDERCOMPONENT_H__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

enum class ColliderType
{
	BOX,
	SPHERE,
	CAPSULE,
	MESH,
	NONE
};

namespace physx
{
	class PxRigidActor;
	class PxRigidStatic;
	class PxTransform;
	class PxShape;
	class PxConvexMesh;
	class PxTriangleMesh;
	class PxVec3;
	//typedef uint16_t PxU16;
}


class ColliderComponent : public Component
{
public:

	ColliderComponent(GameObject* owner, ColliderType type);
	virtual ~ColliderComponent();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return ComponentType::RIGIDBODY; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	void CreateCollider(ColliderType type, bool createAgain = false);

	void DisplayComponentMenu();

	ColliderType type = ColliderType::NONE;

	float3 centerPosition = float3::zero;

	bool IsTrigger() { return isTrigger; }
	void SetTrigger(bool enable) { isTrigger = enable; }

private:

	physx::PxShape* shape = nullptr;
	float3 colliderSize = float3(10, 10, 10);

	float radius = 1.0f;
	float height = 2.0f;
	bool isTrigger = false;
};

#endif //__C_COLLIDERCOMPONENT_H__