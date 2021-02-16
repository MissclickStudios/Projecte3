#ifndef __R_MESH_H__
#define __R_MESH_H__

#include "MathGeoBoundingBox.h"
#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float4x4.h"

#include "Bone.h"

#include "Resource.h"
#include "MeshSettings.h"

class ParsonNode;

struct Bone;
struct BoneWeight;

typedef unsigned int uint;

struct Vertex
{
	float3 position;
	float3 normals;
	float2 tex_coords;
};

class R_Mesh : public Resource
{
public:
	R_Mesh();
	~R_Mesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& meta_root) const override;
	bool LoadMeta(const ParsonNode& meta_root) override;

public:
	void LoadBuffers();
	void SwapBonesToVertexArray();

	AABB GetAABB() const;
	void SetAABB();

public:
	//std::vector<Vertex>		vertices;
	std::vector<float>			vertices;
	std::vector<float>			normals;
	std::vector<float>			tex_coords;
	std::vector<uint>			indices;

	std::vector<Bone>			bones;
	std::vector<BoneWeight>		bone_weights;
	std::vector<float4x4>		bone_offsets;
	std::map<std::string, uint> bone_mapping;
	//std::multimap<uint, BoneWeight>	bone_weights;

	std::vector<float4x4>		bone_transforms;

	// Buffer data
	uint						VBO;												// Vertex Buffer Object.	-->		Will store all the buffer data of the vertices of the mesh.
	uint						NBO;												// Normals Buffer Object.	-->		Will store all the buffer data of the normals of the mesh.
	uint						TBO;												// Tex Coord Buffer Object. -->		Will store all the buffer data of the tex coords of the mesh.
	uint						IBO;												// Index Buffer Object.		--> 	Will store all the buffer data of the indices of the mesh.

	bool						draw_vertex_normals;
	bool						draw_face_normals;

	AABB						aabb;

private:
	MeshSettings				mesh_settings;

};

#endif // !__R_MESH_H__
