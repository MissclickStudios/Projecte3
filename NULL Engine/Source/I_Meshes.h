#ifndef __I_MESHES_H__
#define __I_MESHES_H__

#include <vector>

#include "MathGeoLib/include/Math/float4x4.h"

#include "Assimp.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiBone;

class R_Mesh;
struct Bone;
struct VertexWeight;

typedef unsigned int uint;

enum class HeaderData
{
	VERTICES,
	NORMALS,
	TEX_COORDS,
	INDICES,
	BONES,
	BONE_DATA_SIZE
};

namespace Importer
{
	namespace Meshes
	{
		void Import	(const aiMesh* assimpMesh, R_Mesh* rMesh);													// Loads all mesh data from the given aiNode & stores it in a new R_Mesh.
		uint Save	(const R_Mesh* rMesh, char** buffer);													// Processes R_Mesh data into ready-to-save buffer. Will return the buffer's size.
		bool Load	(const char* buffer, R_Mesh* rMesh);													// Processes buffer data into a ready-to-use R_Mesh. Will return nullptr on ERROR.

		namespace Utilities
		{
			void GetVertices			(const aiMesh* assimpMesh, R_Mesh* rMesh);								// Gets the data for the meshes' vertex positions.
			void GetNormals				(const aiMesh* assimpMesh, R_Mesh* rMesh);								// Gets the data for the meshes' vertex normals.
			void GetTexCoords			(const aiMesh* assimpMesh, R_Mesh* rMesh);								// Gets the data for the meshes' vertex tex coords.
			void GetIndices				(const aiMesh* assimpMesh, R_Mesh* rMesh);								// Gets the data for the meshes' indices.

			void GetBones				(const aiMesh* assimpMesh, R_Mesh* rMesh);								// Gets the data for the mesh's bones.
			void GetOffsetMatrix		(const aiBone* assimpBone, float4x4& offsetMatrix);						// Gets the offset matrix of the given bone.
			void GetWeights				(const aiBone* assimpBone, std::vector<VertexWeight>& weights);			// Gets the Weights of a given bone.

			uint GetBonesDataSize		(const R_Mesh* rMesh);												// Calculates the data size for a given Bones vector.
			void StoreBoneName			(const Bone& bone, char** cursor);									// Stores the given bone's name in the given buffer.
			void StoreBoneOffsetMatrix	(const Bone& bone, char** cursor);									// Stores the given bone's offset matrix in the given buffer
			void StoreBoneWeights		(const Bone& bone, char** cursor);									// Stores the given bone's weights in the given buffer

			void LoadBoneName			(char** cursor, Bone& bone);										// Loads the given bone's name from the given buffer.
			void LoadBoneOffsetMatrix	(char** cursor, Bone& bone);										// Loads the given bone's offset matrix from the given buffer.
			void LoadBoneWeights		(char** cursor, Bone& bone);										// Loads the given bone's weights from the given buffer.
		}
	}
}

#endif // !__I_MESHES_H__