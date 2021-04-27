#ifndef __C_CAPSULECOLLIDERCOMPONENT_H__
#define __C_CAPSULECOLLIDERCOMPONENT_H__

#include "Component.h"

#include "Collider.h"

#include "MathGeoLib/include/Math/float3.h"

class MISSCLICK_API C_CapsuleCollider : public Component
{
public:

	C_CapsuleCollider(GameObject* owner);
	virtual ~C_CapsuleCollider();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return ComponentType::CAPSULE_COLLIDER; } // This is needed to be able to use templates for functions such as GetComponent<>();

	bool IsTrigger() const { return isTrigger; }
	void SetTrigger(bool enable) { isTrigger = enable; ToUpdate(ColliderUpdateType::STATE); }

	const float3	GetCenter() const { return centerPosition; }
	void			SetCenter(float x, float y, float z) { centerPosition = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE); }

	const float		Radius() const { return radius; }
	void			SetRadius(float r) { radius = r; ToUpdate(ColliderUpdateType::SHAPE); }
	const float		Height() const { return height; }
	void			SetHeight(float h) { height = h; ToUpdate(ColliderUpdateType::SHAPE); }

	void UpdateFilter() { ToUpdate(ColliderUpdateType::STATE); }

private:

	void CreateCollider();

	physx::PxShape* shape = nullptr;

	bool isTrigger = false;
	float3 centerPosition = float3::zero;
	float radius = 1.0f;
	float height = 2.0f;

	ColliderUpdateType toUpdate = ColliderUpdateType::NONE;
	void ToUpdate(ColliderUpdateType update);
};

#endif // !__C_CAPSULECOLLIDERCOMPONENT_H__