#ifndef __C_SPHERECOLLIDERCOMPONENT_H__
#define __C_SPHERECOLLIDERCOMPONENT_H__

#include "Component.h"

#include "Collider.h"

#include "MathGeoLib/include/Math/float3.h"

class NULL_API C_SphereCollider : public Component
{
public:

	C_SphereCollider(GameObject* owner);
	virtual ~C_SphereCollider();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return ComponentType::SPHERE_COLLIDER; } // This is needed to be able to use templates for functions such as GetComponent<>();

	bool IsTrigger() const { return isTrigger; }
	void SetTrigger(bool enable) { isTrigger = enable; ToUpdate(ColliderUpdateType::STATE); }

	const float3	GetCenter() const { return centerPosition; }
	void			SetCenter(float x, float y, float z) { centerPosition = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE); }

	const float		Radius() const { return radius; }
	void			SetRadius(float r) { radius = r; ToUpdate(ColliderUpdateType::SHAPE); }

private:

	void CreateCollider();

	physx::PxShape* shape = nullptr;

	bool isTrigger = false;
	float3 centerPosition = float3::zero;
	float radius = 1.0f;

	ColliderUpdateType toUpdate = ColliderUpdateType::NONE;
	void ToUpdate(ColliderUpdateType update);
};

#endif // !__C_SPHERECOLLIDERCOMPONENT_H__