#ifndef __I_MESHES_H__
#define __I_MESHES_H__

#include <vector>
#include <map>

#include "MathGeoLib/include/Math/float4x4.h"

#include "Assimp.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiBone;

class R_Mesh;
struct Bone;
struct VertexWeight;
struct BoneWeight;

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
		void Import	(const aiMesh* assimpMesh, R_Mesh* rMesh);												// Loads all mesh data from the given aiNode & stores it in a new R_Mesh.
		uint Save	(const R_Mesh* rMesh, char** buffer);													// Processes R_Mesh data into ready-to-save buffer. Will return the buffer's size.
		bool Load	(const char* buffer, R_Mesh* rMesh);													// Processes buffer data into a ready-to-use R_Mesh. Will return nullptr on ERROR.

		namespace Utilities
		{
			void GetVertices			(const aiMesh* assimpMesh, R_Mesh* rMesh);							// Gets the data for the meshes' vertex positions.
			void GetNormals				(const aiMesh* assimpMesh, R_Mesh* rMesh);							// Gets the data for the meshes' vertex normals.
			void GetTexCoords			(const aiMesh* assimpMesh, R_Mesh* rMesh);							// Gets the data for the meshes' vertex tex coords.
			void GetIndices				(const aiMesh* assimpMesh, R_Mesh* rMesh);							// Gets the data for the meshes' indices.

			void GetBones				(const aiMesh* assimpMesh, R_Mesh* rMesh);											// Gets the data for the mesh's bones.
			void GetOffsetMatrix		(const aiBone* assimpBone, float4x4& offsetMatrix);									// Gets the offset matrix of the given bone.
			void GetWeights				(const aiBone* assimpBone, uint boneID, std::multimap<uint, BoneWeight>& weights);	// Gets the Weights of a given bone.
			void SetBoneIDsAndWeights	(const std::multimap<uint, BoneWeight>& weights, R_Mesh* rMesh);					// Stores the IDs and Weights in the correct format in the mesh.

			uint GetBonesDataSize		(const R_Mesh* rMesh);												// Calculates the data size for a given Bones vector.
			uint GetBoneMappingSize		(const std::map<std::string, uint>& boneMapping);
			
			void SaveBoneMapping		(char** cursor, const std::map<std::string, uint>& boneMapping);	// Stores the given bone's name in the given buffer.
			void LoadBoneMapping		(char** cursor, R_Mesh* rMesh,uint bonesSize);						// Loads the given bone's name from the given buffer.
			void LoadBoneOffset			(char** cursor, R_Mesh* rMesh, uint bonesSize);						// Loads the given bone's offset matrix from the given buffer.
		}
	}
}

#endif // !__I_MESHES_H__