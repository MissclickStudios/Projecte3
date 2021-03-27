#ifndef __BONE_H__
#define __BONE_H__

#include "MathGeoLib/include/Math/float4x4.h"

#define INVALID_VERTEX_ID	4294967295																// TMP
#define INVALID_BONE_ID		4294967295																// TMP. MAX POSSIBLE VALUE FOR AN UINT.
#define INVALID_WEIGHT		2.0f																	// INVALID AS THE TOTAL VALUE OF ALL 4 POSSIBLE WEIGHTS ALWAYS HAS TO BE 1.0f.

typedef unsigned int uint;

struct VertexWeight
{
	VertexWeight() : vertexId(INVALID_VERTEX_ID), weight(0.0f) {}
	VertexWeight(uint vertexId, float weight) : vertexId(vertexId), weight(weight) {}

	uint	vertexId;
	float	weight;
};

struct BoneWeight
{
	BoneWeight() : boneId(INVALID_BONE_ID), weight(0.0f) {}
	BoneWeight(uint boneId, float weight) : boneId(boneId), weight(weight) {}

	uint boneId;
	float weight;
};

struct Bone
{
	Bone() : name("[NONE]"), offsetMatrix(float4x4::identity) {}
	Bone(const std::string& name, const float4x4& offsetMatrix, const std::vector<VertexWeight>& weights) : name(name), offsetMatrix(offsetMatrix), weights(weights) {}

	std::string					name;
	float4x4					offsetMatrix;
	std::vector<VertexWeight>	weights;
};

#endif // !__BONE_H__