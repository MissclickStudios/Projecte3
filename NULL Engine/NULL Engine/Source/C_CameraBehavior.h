#ifndef __C_CAMERABEHAVIOR__
#define __C_CAMERABEHAVIOR__

#include "Component.h"

#include "MathGeoLib/include/Math/float3.h"

class GameObject;

class C_CameraBehavior : public Component
{
public:

	C_CameraBehavior(GameObject* owner);
	~C_CameraBehavior();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::CAMERA_BEHAVIOR; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	const float3 GetOffset() const { return offset; }
	void SetOffset(float3 offset) { this->offset = offset; }

private:

	float3 offset = float3::zero;
	GameObject* player = nullptr;
};

#endif
