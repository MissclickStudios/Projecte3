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
	if (assimpMesh->HasPositions())
		Utilities::GetVertices(assimpMesh, rMesh);														// Gets the vertices data stored in the given ai_mesh.

	if (assimpMesh->HasNormals())
		Utilities::GetNormals(assimpMesh, rMesh);															// Gets the normals data stored in the given ai_mesh

	if (assimpMesh->HasTextureCoords(0))
		Utilities::GetTexCoords(assimpMesh, rMesh);														// Gets the tex coords data stored in the given ai_mesh.

	if (assimpMesh->HasFaces())																		// Double checked if for whatever reason Generate Mesh is called independently.
		Utilities::GetIndices(assimpMesh, rMesh);															// Gets the indices data stored in the given ai_mesh.

	// Loading and reorganizing the bone data from the mesh
	if (assimpMesh->HasBones())
		Utilities::GetBones(assimpMesh, rMesh);

	//rMesh->SwapBonesToVertexArray();

	rMesh->LoadBuffers();																			// 
	rMesh->SetAABB();																				// 
}

void Importer::Meshes::Utilities::GetVertices(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	uint verticesSize = assimpMesh->mNumVertices * 3;													// There will be 3 coordinates per vertex, hence the size will be numVertices * 3.
	rMesh->vertices.resize(verticesSize);															// Allocating in advance the memory required to store all the verts.

	memcpy(&rMesh->vertices[0], assimpMesh->mVertices, sizeof(float) * verticesSize);				// &rMesh->vertices[0] gets a pointer to the beginning of the vector.

	LOG("[STATUS] Imported %u position vertices!", verticesSize);
}

void Importer::Meshes::Utilities::GetNormals(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
	uint normalsSize = assimpMesh->mNumVertices * 3;													// There will be 3 normal coordinates per vertex.
	rMesh->normals.resize(normalsSize);															// Allocating in advance the memory required to store all the normals.

	memcpy(&rMesh->normals[0], assimpMesh->mNormals, sizeof(float) * normalsSize);

	LOG("[STATUS] Imported %u normals!", normalsSize);
}

void Importer::Meshes::Utilities::GetTexCoords(const aiMesh* assimpMesh, R_Mesh* rMesh)
{	
	uint texCoordsSize = assimpMesh->mNumVertices * 2;												// There will be 2 tex coordinates per vertex.
	rMesh->texCoords.resize(texCoordsSize);														// Allocating in advance the memory required to store all the texture coordinates.

	for (uint i = 0; i < assimpMesh->mNumVertices; ++i)
	{
		rMesh->texCoords[i * 2]		= assimpMesh->mTextureCoords[0][i].x;
		rMesh->texCoords[i * 2 + 1]	= assimpMesh->mTextureCoords[0][i].y;
	}

	LOG("[STATUS] Imported %u texture coordinates!", texCoordsSize);
}

void Importer::Meshes::Utilities::GetIndices(const aiMesh* assimpMesh, R_Mesh* rMesh)
{
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
	rMesh->isSkinned = true;

	rMesh->boneIDs = new int[assimpMesh->mNumVertices * 4];		//Usually between 1 and 4
	rMesh->boneWeights = new float[assimpMesh->mNumVertices * 4];

	for (uint i = 0; i < assimpMesh->mNumBones; ++i)
	{
		aiBone* assimpBone = assimpMesh->mBones[i];
		std::string name = assimpBone->mName.C_Str();
		
		rMesh->boneMapping.emplace(name, i);

		float4x4 offsetMatrix = float4x4::identity;
		Utilities::GetOffsetMatrix(assimpBone, offsetMatrix);
		rMesh->boneOffsets.push_back(offsetMatrix);

		for (uint j = 0; j < assimpBone->mNumWeights; ++j)
		{
			rMesh->boneIDs[i + j] = assimpBone->mWeights[j].mVertexId;
			rMesh->boneWeights[i + j] = assimpBone->mWeights[j].mWeight;
		}
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
	
}

uint Importer::Meshes::Utilities::GetBonesDataSize(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Mesh Importer: Could not Get Bone Data Size! Error: Given R_Mesh* was nullptr.");
		return 0;
	}
	
	uint size = 0;

	size += sizeof(int) * rMesh->vertices.size()/3 * 4 + sizeof(float) * rMesh->vertices.size()/3 * 4
		+ sizeof(uint) * rMesh->boneMapping.size() * 2	//Store bone names size and boneIds
		+ sizeof(float4x4) * rMesh->boneOffsets.size();
	
	for (std::map<std::string, uint>::const_iterator bone = rMesh->boneMapping.begin(); bone != rMesh->boneMapping.end(); bone++)
	{
		size += bone->first.size() * sizeof(char);
	}
	
	return size;
}

void Importer::Meshes::Utilities::SaveBoneMapping(char** cursor,const R_Mesh* rMesh)
{
	uint bytes = 0;

	for (std::map<std::string, uint>::const_iterator bone = rMesh->boneMapping.begin(); bone != rMesh->boneMapping.end(); ++bone)
	{
		uint nameLength[1] = { bone->first.length() };

		bytes = sizeof(uint);
		memcpy(*cursor, &nameLength[0], bytes);
		*cursor += bytes;

		bytes = bone->first.length() * sizeof(char);
		memcpy(*cursor, bone->first.c_str(), bytes);
		*cursor += bytes;

		bytes = sizeof(uint);
		memcpy(*cursor, &bone->second, bytes);
		*cursor += bytes;

		LOG("Mapping %s , %d", bone->first.c_str(), bone->second);
	}
}

void Importer::Meshes::Utilities::SaveBoneOffset(char** cursor, const R_Mesh* rMesh)
{
	uint bytes = sizeof(float4x4) * rMesh->boneOffsets.size();
	memcpy(*cursor, &rMesh->boneOffsets[0], bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneMapping(char** cursor, R_Mesh* rMesh, uint bonesSize)
{
	uint nameLength = 0;
	uint bytes = 0;

	for (uint i =0; i < bonesSize;i++)
	{
		std::string name;
		uint uid = 0;

		bytes = sizeof(uint);
		memcpy(&nameLength ,*cursor, bytes);
		*cursor += bytes;

		name.resize(nameLength);
		bytes = nameLength * sizeof(char);
		memcpy(&name[0] ,*cursor , bytes); //problem loading here
		*cursor += bytes;

		bytes = sizeof(uint);
		memcpy(&uid, *cursor, bytes);
		*cursor += bytes;

		rMesh->boneMapping.emplace(name, uid);
	}
}

void Importer::Meshes::Utilities::LoadBoneOffset(char** cursor, R_Mesh* rMesh, uint bonesSize)
{
	uint bytes = sizeof(float4x4) * bonesSize;
	rMesh->boneOffsets.resize(bonesSize);
	memcpy(&rMesh->boneOffsets[0] ,*cursor , bytes);
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
		rMesh->texCoords.size(), 																	// 2 --> Num Texture Coordinates
		rMesh->indices.size(), 																	// 3 --> Num Indices
		rMesh->boneMapping.size()																	// 4 --> Num Bones
	};

	uint headerDataSize			= sizeof(headerData) + sizeof(uint);
	uint arrayDataSize			= (headerData[0] + headerData[1] + headerData[2]) * sizeof(float) + headerData[3] * sizeof(uint);
	
	uint bonesDataSize = 0;

	if(rMesh->isSkinned)
		bonesDataSize = Utilities::GetBonesDataSize(rMesh);

	uint precalculatedDataSize	= rMesh->aabb.NumVertices() * sizeof(float) * 3;

	uint size = headerDataSize + arrayDataSize + bonesDataSize + precalculatedDataSize;

	if (size == 0)
	{
		LOG("[WARNING] Mesh {%s} with UID [%u] had no data to Save!", rMesh->GetAssetsFile(), rMesh->GetUID());
		return 0;
	}

	*buffer	= new char[size];
	char* cursor = *buffer;
	uint bytes	= 0;

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

	if (rMesh->texCoords.size() != 0)
	{
		bytes = rMesh->texCoords.size() * sizeof(float);
		memcpy_s(cursor, size, &rMesh->texCoords[0], bytes);
		cursor += bytes;
	}

	bytes = rMesh->indices.size() * sizeof(uint);
	memcpy_s(cursor, size, &rMesh->indices[0], bytes);
	cursor += bytes;

	// --- BONE DATA ---
	if (rMesh->isSkinned)
	{
		uint nVertices = rMesh->vertices.size() / 3 * 4;

		bytes = nVertices * sizeof(uint);
		memcpy_s(cursor, size, &rMesh->boneIDs[0], bytes);
		cursor += bytes;

		for (int i = 0; i < 10; i++)
		{
			LOG("boneIDs %d %d",i,rMesh->boneIDs[i]);
		}

		bytes = nVertices * sizeof(float);
		memcpy_s(cursor, size, &rMesh->boneWeights[0], bytes);
		cursor += bytes;

		for (int i = 0; i < 10; i++)
		{
			LOG("boneIDs %d %f", i, rMesh->boneWeights[i]);
		}

		Utilities::SaveBoneMapping(&cursor,rMesh);
		Utilities::SaveBoneOffset(&cursor, rMesh);
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

	rMesh->texCoords.resize(headerData[2]);
	if (rMesh->texCoords.size() != 0)
	{
		bytes = headerData[2] * sizeof(float);
		memcpy_s(&rMesh->texCoords[0], bytes, cursor, bytes);
		cursor += bytes;
	}

	rMesh->indices.resize(headerData[3]);
	bytes = headerData[3] * sizeof(uint);
	memcpy_s(&rMesh->indices[0], bytes, cursor, bytes);
	cursor += bytes;

	// ---BONE DATA ---
	uint nVertices = rMesh->vertices.size() / 3 * 4;

	rMesh->boneIDs = new int[nVertices];
	rMesh->boneWeights = new float[nVertices];

	bytes = nVertices * sizeof(uint);
	memcpy_s(rMesh->boneIDs,bytes,cursor, bytes);
	cursor += bytes;
	for (int i = 0; i < 10; i++)
	{
		LOG("boneIDs %d %d", i, rMesh->boneIDs[i]);
	}
	bytes = nVertices * sizeof(float);
	memcpy_s(rMesh->boneWeights, bytes, cursor, bytes);
	cursor += bytes;
	for (int i = 0; i < 10; i++)
	{
		LOG("boneIDs %d %f", i, rMesh->boneWeights[i]);
	}
	//bone mapping
	Utilities::LoadBoneMapping(&cursor, rMesh, headerData[4]);

	//boneOffsets
	Utilities::LoadBoneOffset(&cursor, rMesh, headerData[4]);

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