#ifndef __RE_CUBOID_H__
#define __RE_CUBOID_H__

#include "MathGeoLib/include/Math/float3.h"

#include "Renderer.h"

enum class Cuboid_Type
{
	NONE,
	AABB,
	OBB,
	FRUSTUM,
	COLLIDER
};

class RE_Cuboid : public Renderer
{
public:
	RE_Cuboid(float4x4* transform, float lineWidth, Color color, const float3* vertices, Cuboid_Type cuboidType);
	RE_Cuboid(const float3* vertices, Cuboid_Type cuboidType);
	~RE_Cuboid();

	bool Render() override;
	bool CleanUp() override;

private:
	Color GetColorByType();
	float GetEdgeWidthByType();

private:
	const float3*	vertices;
	Cuboid_Type		cuboidType;
};

#endif // !__RE_CUBOID_H__