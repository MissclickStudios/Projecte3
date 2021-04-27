#ifndef __R_MESH_H__
#define __R_MESH_H__

#include <map>

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

class MISSCLICK_API R_Mesh : public Resource
{
public:
	R_Mesh();
	~R_Mesh();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

	static inline ResourceType GetType() { return ResourceType::MESH; }

public:
	void LoadStaticBuffers		();
	void LoadSkinningBuffers	(bool initStatic = false);

	AABB GetAABB				() const;
	void SetAABB				();

public:
	std::vector<float>			vertices;
	std::vector<float>			normals;
	std::vector<float>			texCoords;
	std::vector<uint>			indices;

	std::vector<uint>			boneIDs;
	std::vector<float>			boneWeights;
	std::vector<float4x4>		boneOffsets;
	std::map<std::string, uint> boneMapping;

	// Buffer data
	uint VAO = 0;
	uint VBO = 0;												// Vertex Buffer Object.	-->		Will store all the buffer data of the vertices of the mesh.
	uint NBO = 0;												// Normals Buffer Object.	-->		Will store all the buffer data of the normals of the mesh.
	uint TBO = 0;												// Tex Coord Buffer Object. -->		Will store all the buffer data of the tex coords of the mesh.
	uint IBO = 0;												// Index Buffer Object.		--> 	Will store all the buffer data of the indices of the mesh.
	uint BBO = 0;
	uint WBO = 0;

	bool drawVertexNormals;
	bool drawFaceNormals;
	bool hasBones;

	AABB aabb;

private:
	MeshSettings meshSettings;
};

#endif // !__R_MESH_H__
