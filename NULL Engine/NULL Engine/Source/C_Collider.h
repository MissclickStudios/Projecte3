#ifndef __C_COLLIDERCOMPONENT_H__
#define __C_COLLIDERCOMPONENT_H__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class C_Collider : public Component
{
public:

	C_Collider(GameObject* owner, ColliderType type);
	virtual ~C_Collider();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return GetComponentType(); } // This is needed to be able to use templates for functions such as GetComponent<>();

	const ColliderType GetColliderType() const { return colType; }

	bool IsTrigger() const { return isTrigger; }
	void SetTrigger(bool enable) { isTrigger = enable; ToUpdate(ColliderUpdateType::STATE); }

	const float3	Size() const { return colliderSize; }
	void			SetSize(float x, float y, float z) { colliderSize = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE);}
	const float3	GetCenter() const { return centerPosition; }
	void			SetCenter(float x, float y, float z) { centerPosition = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE); }

	const float		Radius() const { return radius; }
	void			SetRadius(float r) { if (colType != ColliderType::BOX) { radius = r; ToUpdate(ColliderUpdateType::SHAPE); } }
	const float		Height() const { return height; }
	void			SetHeight(float h) { if (colType == ColliderType::CAPSULE) { height = h; ToUpdate(ColliderUpdateType::SHAPE); } }

private:

	ComponentType GetComponentType(ColliderType type);
	void CreateCollider(ColliderType type);

	ColliderType colType = ColliderType::NONE;

	physx::PxShape* shape = nullptr;

	bool isTrigger = false;
	float3 colliderSize = float3(10, 10, 10);
	float3 centerPosition = float3::zero;
	float radius = 1.0f;
	float height = 2.0f;

	ColliderUpdateType toUpdate = ColliderUpdateType::NONE;
	void ToUpdate(ColliderUpdateType update);
};

#endif //__C_COLLIDERCOMPONENT_H__