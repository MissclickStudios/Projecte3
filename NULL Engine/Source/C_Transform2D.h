#ifndef __C_TRANSFORM2D_H__
#define __C_TRANSFORM2D_H__

#include <vector>

#include "Component.h"
#include "UIElement.h"

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/Quat.h"

class ParsonNode;
class GameObject;

class C_Transform2D : public Component
{
public:
	C_Transform2D(GameObject* owner);
	~C_Transform2D();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::TRANSFORM2D; }

	float3 QuaternionToEuler(Quat q);
	Quat EulerToQuaternion(float3 eu);

	Quat LookAt(const float3& point);

	float2 CalculateMovement(float4x4 matrix, float2 goal);

public:

	float2 GetPosition()const;
	float3 GetRotation()const;
	float2 GetSize()const;
	float4x4 GetGlobalMatrix() const;
	float4x4 GetGlobalMatrixTransposed() const;

	void SetTransformation(float3 pos, Quat rot, float2 size, float3 pivot);
	void SetPivot(float3 pivot);
	void SetPosition(float2 pos);
	void SetRotation(Quat rot);
	void SetRotation(float3 rot);
	void SetSize(float2 size);
	void UpdateZ();
	bool UpdateMatrixBillboard();
	void SetMatrices();
	void SetMatricesWithNewParent(float4x4 parent_global_matrix);

	void GetTrianglePoints(float2& min_p, float2& max_p, float2& third_p, float2& third_p_second_tri);

	float GetDepth();

public:

	bool needed_to_update = false;
	bool needed_to_update_only_children = false;

private:

	float4x4 local_matrix;
	float4x4 matrix_billboard;
	float4x4 global_matrix;
	float4x4 matrix_pivot;
	float4x4 matrix_parent;
	float4x4 global_matrix_transposed;
	float2 position;
	float z_depth;
	float z_depth_with_layers;
	float2 pivot_position = { 0,0 };
	float2 size;
	float2 relative_size;
	float2 relative_position;
	float3 rotation;
};

#endif // !__C_TRANSFORM2D_H__