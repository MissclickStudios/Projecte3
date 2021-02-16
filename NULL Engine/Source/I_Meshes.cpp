// ----------------------------------------------------
// I_Meshes.cpp --- Importing .FBX files into meshes.
// Interface class between the engine and Assimp.
// ----------------------------------------------------

#include "MathGeoTransform.h"
#include "Assimp.h"

#include "Log.h"
#include "VariableTypedefs.h"

#include "Bone.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "R_Mesh.h"

#include "I_Meshes.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/Assimp/libx86/assimp.lib")

#define HEADER_SIZE 5																				// Amount of items in the Header Data Array. (Save & Load)

using namespace Importer::Meshes;																	// Not a good thing to do but it will be employed sparsely and only inside this .cpp

void Importer::Meshes::Import(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Mesh! Error: R_Mesh* was nullptr.");
		return;
	}
	if (assimpMesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Mesh { %s }! Error: aiMesh* was nullptr.", rMesh->GetAssetsFile());
		return;
	}
	
	// Loading the data from the mesh into the corresponding vectors
	Utilities::GetVertices(assimpMesh, rMesh);														// Gets the vertices data stored in the given ai_mesh.
	Utilities::GetNormals(assimpMesh, rMesh);															// Gets the normals data stored in the given ai_mesh
	Utilities::GetTexCoords(assimpMesh, rMesh);														// Gets the tex coords data stored in the given ai_mesh.
	Utilities::GetIndices(assimpMesh, rMesh);															// Gets the indices data stored in the given ai_mesh.

	// Loading and reorganizing the bone data from the mesh
	std::vector<Bone> bones;
	Utilities::GetBones(assimpMesh, rMesh);
	rMesh->SwapBonesToVertexArray();

	rMesh->LoadBuffers();																			// 
	rMesh->SetAABB();																				// 
}

void Importer::Meshes::Utilities::GetVertices(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	if (!assimpMesh->HasPositions())
	{
		LOG("[ERROR] Imported Mesh has no position vertices!");
		return;
	}

	uint verticesSize = assimpMesh->mNumVertices * 3;													// There will be 3 coordinates per vertex, hence the size will be numVertices * 3.
	rMesh->vertices.resize(verticesSize);															// Allocating in advance the memory required to store all the verts.

	memcpy(&rMesh->vertices[0], assimpMesh->mVertices, sizeof(float) * verticesSize);				// &rMesh->vertices[0] gets a pointer to the beginning of the vector.

	LOG("[STATUS] Imported %u position vertices!", verticesSize);
}

void Importer::Meshes::Utilities::GetNormals(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	if (!assimpMesh->HasNormals())
	{
		LOG("[ERROR] Imported Mesh has no normals!");
		return;
	}

	uint normalsSize = assimpMesh->mNumVertices * 3;													// There will be 3 normal coordinates per vertex.
	rMesh->normals.resize(normalsSize);															// Allocating in advance the memory required to store all the normals.

	memcpy(&rMesh->normals[0], assimpMesh->mNormals, sizeof(float) * normalsSize);

	LOG("[STATUS] Imported %u normals!", normalsSize);
}

void Importer::Meshes::Utilities::GetTexCoords(const aiMesh* assimpMesh, R_Mesh* rMesh)
{	
	if (!assimpMesh->HasTextureCoords(0))
	{
		LOG("[ERROR] Imported Mesh has no tex coords!");
		return;
	}

	uint texCoordsSize = assimpMesh->mNumVertices * 2;												// There will be 2 tex coordinates per vertex.
	rMesh->tex_coords.resize(texCoordsSize);														// Allocating in advance the memory required to store all the texture coordinates.

	for (uint i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		rMesh->tex_coords[i * 2]		= assimpMesh->mTextureCoords[0][i].x;
		rMesh->tex_coords[i * 2 + 1]	= assimpMesh->mTextureCoords[0][i].y;
	}

	LOG("[STATUS] Imported %u texture coordinates!", texCoordsSize);
}

void Importer::Meshes::Utilities::GetIndices(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	if (!assimpMesh->HasFaces())																		// Double checked if for whatever reason Generate Mesh is called independently.
	{
		LOG("[ERROR] Imported Mesh has no faces!");
		return;
	}

	uint indicesSize = assimpMesh->mNumFaces * 3;														// The size of the indices vector will be equal to the amount of faces times 3 (triangles).
	rMesh->indices.resize(indicesSize);															// Allocating in advance the memory required to store all the indices.

	for (uint i = 0; i < assimpMesh->mNumFaces; ++i)
	{
		aiFace face = assimpMesh->mFaces[i];															// Getting the face that is currently being iterated.
		uint numInd = face.mNumIndices;

		if (numInd == 3)
		{
			memcpy(&rMesh->indices[i * numInd], face.mIndices, sizeof(uint) * numInd);			// As each face has 3 elements (vertices), the memcpy will be done 3 vertices at a time.
		}
		else
		{
			LOG("[WARNING] Geometry face %u with != 3 indices!", i);
		}
	}

	LOG("[STATUS] Imported %u mesh indices!", indicesSize);
}

void Importer::Meshes::Utilities::GetBones(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	if (!assimpMesh->HasBones())
	{
		return;
	}
	
	for (uint i = 0; i < assimpMesh->mNumBones; ++i)
	{
		aiBone* assimpBone = assimpMesh->mBones[i];

		std::string name = assimpBone->mName.C_Str();
		
		float4x4 offsetMatrix = float4x4::identity;
		Utilities::GetOffsetMatrix(assimpBone, offsetMatrix);

		std::vector<VertexWeight> weights;
		Utilities::GetWeights(assimpBone, weights);

		rMesh->bones.push_back(Bone(name, offsetMatrix, weights));
	}
}

void Importer::Meshes::Utilities::GetOffsetMatrix(const aiBone* assimpBone, float4x4& offsetMatrix)
{
	aiTransform aiT;
	Transform	maT;

	assimpBone->mOffsetMatrix.Decompose(aiT.position, aiT.rotation, aiT.scale);

	maT.position	= { aiT.position.x, aiT.position.y, aiT.position.z };
	maT.rotation	= { aiT.rotation.x, aiT.rotation.y, aiT.rotation.z, aiT.rotation.w };
	maT.scale		= { aiT.scale.x, aiT.scale.y, aiT.scale.z };

	offsetMatrix = float4x4::FromTRS(maT.position, maT.rotation, maT.scale);
}

void Importer::Meshes::Utilities::GetWeights(const aiBone* assimpBone, std::vector<VertexWeight>& weights)
{
	for (uint j = 0; j < assimpBone->mNumWeights; ++j)
	{
		const aiVertexWeight& aiWeight = assimpBone->mWeights[j];

		uint vertexId	= aiWeight.mVertexId;
		float weight	= aiWeight.mWeight;

		weights.push_back(VertexWeight(vertexId, weight));
	}
}

uint Importer::Meshes::Utilities::GetBonesDataSize(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Mesh Importer: Could not Get Bone Data Size! Error: Given R_Mesh* was nullptr.");
		return 0;
	}
	
	uint bonesSize = 0;

	for (auto bone = rMesh->bones.cbegin(); bone != rMesh->bones.cend(); ++bone)
	{
		uint boneSize = 0;

		boneSize += (bone->name.length() * sizeof(char)) + sizeof(uint);											// Size of the name + size of the size variable.
		boneSize += 16 * sizeof(float);																			// Any given float4x4 is composed by 16 floats.
		boneSize += (bone->weights.size() * sizeof(VertexWeight)) + sizeof(uint);									// Size of each weight + size of the size variable.

		bonesSize += boneSize;
	}

	return bonesSize;
}

void Importer::Meshes::Utilities::StoreBoneName(const Bone& bone, char** cursor)
{
	uint bytes			= 0;
	uint nameLength	= bone.name.length();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&nameLength, bytes);
	*cursor += bytes;

	bytes = bone.name.length() * sizeof(char);
	memcpy(*cursor, (const void*)bone.name.c_str(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::StoreBoneOffsetMatrix(const Bone& bone, char** cursor)
{
	uint bytes = 16 * sizeof(float);
	memcpy(*cursor, (const void*)bone.offsetMatrix.ptr(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::StoreBoneWeights(const Bone& bone, char** cursor)
{
	uint bytes			= 0;
	uint weightsSize	= bone.weights.size();
	
	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&weightsSize, bytes);
	*cursor += bytes;

	bytes = bone.weights.size() * sizeof(VertexWeight);
	memcpy(*cursor, (const void*)&bone.weights[0], bytes);
	//memcpy(*cursor, (const void*)bone.weights.data(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneName(char** cursor, Bone& bone)
{
	uint bytes			= 0;
	uint nameLength	= 0;

	bytes = sizeof(uint);
	memcpy(&nameLength, *cursor, bytes);
	*cursor += bytes;

	bone.name.resize(nameLength);
	bytes = nameLength * sizeof(char);
	memcpy(&bone.name[0], *cursor, bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneOffsetMatrix(char** cursor, Bone& bone)
{
	uint bytes = 16 * sizeof(float);
	memcpy(bone.offsetMatrix.ptr(), *cursor, bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneWeights(char** cursor, Bone& bone)
{
	uint bytes			= 0;
	uint numWeights	= 0;

	bytes = sizeof(uint);
	memcpy(&numWeights, *cursor, bytes);
	*cursor += bytes;

	bone.weights.resize(numWeights);
	bytes = numWeights * sizeof(VertexWeight);
	memcpy(&bone.weights[0], *cursor, bytes);
	*cursor += bytes;
}

uint Importer::Meshes::Save(const R_Mesh* rMesh, char** buffer)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Could not Save Mesh to Library! Error: R_Mesh* was nullptr");
		return 0;
	}
	
	uint written = 0;

	uint headerData[HEADER_SIZE] = {
		rMesh->vertices.size(),																	// 0 --> Num Vertices
		rMesh->normals.size(), 																	// 1 --> Num Normals
		rMesh->tex_coords.size(), 																	// 2 --> Num Texture Coordinates
		rMesh->indices.size(), 																	// 3 --> Num Indices
		rMesh->bones.size()																		// 4 --> Num Bones
	};

	uint headerDataSize			= sizeof(headerData) + sizeof(uint);
	uint arrayDataSize			= (headerData[0] + headerData[1] + headerData[2]) * sizeof(float) + headerData[3] * sizeof(uint);
	uint bonesDataSize			= Utilities::GetBonesDataSize(rMesh);
	uint precalculatedDataSize	= rMesh->aabb.NumVertices() * sizeof(float) * 3;

	uint size = headerDataSize + arrayDataSize + bonesDataSize + precalculatedDataSize;

	if (size == 0)
	{
		LOG("[WARNING] Mesh {%s} with UID [%u] had no data to Save!", rMesh->GetAssetsFile(), rMesh->GetUID());
		return 0;
	}

	*buffer				= new char[size];
	char* cursor		= *buffer;
	uint bytes			= 0;

	// --- HEADER DATA ---
	bytes = sizeof(headerData);
	memcpy_s(cursor, size, headerData, bytes);
	cursor += bytes;

	// --- VERTEX ARRAY DATA ---
	bytes = rMesh->vertices.size() * sizeof(float);
	memcpy_s(cursor, size, &rMesh->vertices[0], bytes);
	cursor += bytes;

	bytes = rMesh->normals.size() * sizeof(float);
	memcpy_s(cursor, size, &rMesh->normals[0], bytes);
	cursor += bytes;

	if (rMesh->tex_coords.size() != 0)
	{
		bytes = rMesh->tex_coords.size() * sizeof(float);
		memcpy_s(cursor, size, &rMesh->tex_coords[0], bytes);
		cursor += bytes;
	}

	bytes = rMesh->indices.size() * sizeof(uint);
	memcpy_s(cursor, size, &rMesh->indices[0], bytes);
	cursor += bytes;

	// --- BONE DATA ---
	for (auto bone = rMesh->bones.cbegin(); bone != rMesh->bones.cend(); ++bone)
	{
		Utilities::StoreBoneName((*bone), &cursor);
		Utilities::StoreBoneOffsetMatrix((*bone), &cursor);
		Utilities::StoreBoneWeights((*bone), &cursor);
	}

	// --- PRECALCULATED DATA ---
	math::vec* aabbCorners = new math::vec[8];
	rMesh->aabb.GetCornerPoints(aabbCorners);

	bytes = rMesh->aabb.NumVertices() * sizeof(float) * 3;
	memcpy_s(cursor, size, aabbCorners, bytes);
	cursor += bytes;

	delete[] aabbCorners;

	// --- SAVING THE BUFFER ---
	std::string path	= MESHES_PATH + std::to_string(rMesh->GetUID()) + MESHES_EXTENSION;
	written				= App->fileSystem->Save(path.c_str(), *buffer, size);

	if (written > 0)
	{	
		LOG("[STATUS] Importer: Successfully Saved Mesh { %s } to Library! Path: %s", rMesh->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Meshes Importer: Could not Save { %s } to Library! Error: File System could not Write File.", rMesh->GetAssetsFile());
	}

	return written;
}

bool Importer::Meshes::Load(const char* buffer, R_Mesh* rMesh)
{
	bool ret = true;

	if (rMesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Mesh from Library! Error: R_Mesh* was nullptr.");
		return false;
	}
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Mesh { %s } from Library! Error: Buffer was nullptr.", rMesh->GetAssetsFile());
		return false;
	}
	
	char* cursor	= (char*)buffer;
	uint bytes		= 0;

	// --- HEADER DATA ---
	uint headerData[HEADER_SIZE];

	bytes = sizeof(headerData);
	memcpy_s(headerData, bytes, cursor, bytes);
	cursor += bytes;

	// --- VERTEX ARRAY DATA ---
	rMesh->vertices.resize(headerData[0]);
	bytes = headerData[0] * sizeof(float);
	memcpy_s(&rMesh->vertices[0], bytes, cursor, bytes);
	cursor += bytes;

	rMesh->normals.resize(headerData[1]);
	bytes = headerData[1] * sizeof(float);
	memcpy_s(&rMesh->normals[0], bytes, cursor, bytes);
	cursor += bytes;

	if (rMesh->tex_coords.size() != 0)
	{
		rMesh->tex_coords.resize(headerData[2]);
		bytes = headerData[2] * sizeof(float);
		memcpy_s(&rMesh->tex_coords[0], bytes, cursor, bytes);
		cursor += bytes;
	}

	rMesh->indices.resize(headerData[3]);
	bytes = headerData[3] * sizeof(uint);
	memcpy_s(&rMesh->indices[0], bytes, cursor, bytes);
	cursor += bytes;

	// ---BONE DATA ---
	rMesh->bones.resize(headerData[4]);
	for (uint i = 0; i < rMesh->bones.size(); ++i)
	{
		Bone bone = Bone();

		Utilities::LoadBoneName(&cursor, bone);
		Utilities::LoadBoneOffsetMatrix(&cursor, bone);
		Utilities::LoadBoneWeights(&cursor, bone);

		rMesh->bones[i] = bone;
	}

	// --- PRECALCULATED DATA ---
	float3 aabbCorners[8];

	bytes = sizeof(aabbCorners);
	memcpy_s(aabbCorners, bytes, cursor, bytes);
	cursor += bytes;

	rMesh->aabb = AABB(aabbCorners[0], aabbCorners[7]);
	rMesh->LoadBuffers();																									// STORING VBO, NBO, TBO, IBO DATA IS IRRELEVANT IN THIS CASE

	LOG("[STATUS] Importer: Successfully Loaded Mesh { %s } from Library!", rMesh->GetAssetsFile());

	return ret;
}