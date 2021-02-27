#ifndef __C_BOXCOLLIDERCOMPONENT_H__
#define __C_BOXCOLLIDERCOMPONENT_H__

#include "Component.h"

#include "Collider.h"

#include "MathGeoLib/include/Math/float3.h"

class C_BoxCollider : public Component
{
public:

	C_BoxCollider(GameObject* owner);
	virtual ~C_BoxCollider();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	void SetIsActive(bool setTo) override;

	static inline ComponentType GetType() { return ComponentType::BOX_COLLIDER; } // This is needed to be able to use templates for functions such as GetComponent<>();

	bool IsTrigger() const { return isTrigger; }
	void SetTrigger(bool enable) { isTrigger = enable; ToUpdate(ColliderUpdateType::STATE); }

	const float3	Size() const { return colliderSize; }
	void			SetSize(float x, float y, float z) { colliderSize = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE);}
	const float3	GetCenter() const { return centerPosition; }
	void			SetCenter(float x, float y, float z) { centerPosition = { x,y,z }; ToUpdate(ColliderUpdateType::SHAPE); }

private:

	void CreateCollider();

	physx::PxShape* shape = nullptr;

	bool isTrigger = false;
	float3 colliderSize = float3(1, 1, 1);
	float3 centerPosition = float3::zero;

	ColliderUpdateType toUpdate = ColliderUpdateType::NONE;
	void ToUpdate(ColliderUpdateType update);
};

#endif // !__C_BOXCOLLIDERCOMPONENT_H__