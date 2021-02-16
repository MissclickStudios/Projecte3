#ifndef __BONE_H__
#define __BONE_H__

#include "MathGeoLib/include/Math/float4x4.h"

#define INVALID_VERTEX_ID 4294967295																// TMP
#define INVALID_BONE_ID 4294967295																	// TMP

typedef unsigned int uint;

struct VertexWeight
{
	VertexWeight() : vertex_id(INVALID_VERTEX_ID), weight(0.0f) {}
	VertexWeight(uint vertex_id, float weight) : vertex_id(vertex_id), weight(weight) {}

	uint	vertex_id;
	float	weight;
};

struct BoneWeight
{
	BoneWeight() : bone_id(INVALID_BONE_ID), weight(0.0f) {}
	BoneWeight(uint bone_id, float weight) : bone_id(bone_id), weight(weight) {}

	uint bone_id;
	float weight;
};

struct Bone
{
	Bone() : name("[NONE]"), offset_matrix(float4x4::identity) {}
	Bone(const std::string& name, const float4x4& offset_matrix, const std::vector<VertexWeight>& weights) : name(name), offset_matrix(offset_matrix), weights(weights) {}

	std::string					name;
	float4x4					offset_matrix;
	std::vector<VertexWeight>	weights;
};

#endif // !__BONE_H__