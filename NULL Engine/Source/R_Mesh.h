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

class NULL_API R_Mesh : public Resource
{
public:
	R_Mesh();
	~R_Mesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:
	void LoadBuffers();
	void SwapBonesToVertexArray();

	AABB GetAABB() const;
	void SetAABB();

public:
	//std::vector<Vertex>		vertices;
	std::vector<float>			vertices;
	std::vector<float>			normals;
	std::vector<float>			texCoords;
	std::vector<uint>			indices;

	int* boneIDs = nullptr;
	float* boneWeights = nullptr;
	std::vector<float4x4>		boneOffsets;
	std::map<std::string, uint> boneMapping;

	// Buffer data
	uint VAO = 0;
	uint VBO = 0;												// Vertex Buffer Object.	-->		Will store all the buffer data of the vertices of the mesh.
	uint NBO = 0;												// Normals Buffer Object.	-->		Will store all the buffer data of the normals of the mesh.
	uint TBO = 0;												// Tex Coord Buffer Object. -->		Will store all the buffer data of the tex coords of the mesh.
	uint IBO = 0;												// Index Buffer Object.		--> 	Will store all the buffer data of the indices of the mesh.

	bool isSkinned = false;
	bool drawVertexNormals = false;
	bool drawFaceNormals = false;

	AABB aabb;

private:
	MeshSettings meshSettings;

};

#endif // !__R_MESH_H__
