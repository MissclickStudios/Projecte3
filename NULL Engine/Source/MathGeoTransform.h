#ifndef __MATH_STRUCTURES_H__
#define __MATH_STRUCTURES_H__

#include "MathGeoLib/include/Math/TransformOps.h"
#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/float3x3.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

struct Transform
{
	Transform() : position(float3::zero), rotation(Quat::identity), scale(float3::one)	{}
	Transform(const float3& position, const Quat& rotation, const float3& scale) : position(position), rotation(rotation), scale(scale) {}
	Transform(float4x4 transform_matrix) { transform_matrix.Decompose(position, rotation, scale); }

	float3	position;
	Quat	rotation;
	float3	scale;
};

#endif // !__MATH_STRUCTURES_H__