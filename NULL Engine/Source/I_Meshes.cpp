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

void Importer::Meshes::Import(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{
	if (r_mesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Mesh! Error: R_Mesh* was nullptr.");
		return;
	}
	if (ai_mesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Mesh { %s }! Error: aiMesh* was nullptr.", r_mesh->GetAssetsFile());
		return;
	}
	
	// Loading the data from the mesh into the corresponding vectors
	Utilities::GetVertices(ai_mesh, r_mesh);														// Gets the vertices data stored in the given ai_mesh.
	Utilities::GetNormals(ai_mesh, r_mesh);															// Gets the normals data stored in the given ai_mesh
	Utilities::GetTexCoords(ai_mesh, r_mesh);														// Gets the tex coords data stored in the given ai_mesh.
	Utilities::GetIndices(ai_mesh, r_mesh);															// Gets the indices data stored in the given ai_mesh.

	// Loading and reorganizing the bone data from the mesh
	std::vector<Bone> bones;
	Utilities::GetBones(ai_mesh, r_mesh);
	r_mesh->SwapBonesToVertexArray();

	r_mesh->LoadBuffers();																			// 
	r_mesh->SetAABB();																				// 
}

void Importer::Meshes::Utilities::GetVertices(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{
	if (!ai_mesh->HasPositions())
	{
		LOG("[ERROR] Imported Mesh has no position vertices!");
		return;
	}

	uint vertices_size = ai_mesh->mNumVertices * 3;													// There will be 3 coordinates per vertex, hence the size will be numVertices * 3.
	r_mesh->vertices.resize(vertices_size);															// Allocating in advance the memory required to store all the verts.

	memcpy(&r_mesh->vertices[0], ai_mesh->mVertices, sizeof(float) * vertices_size);				// &r_mesh->vertices[0] gets a pointer to the beginning of the vector.

	LOG("[STATUS] Imported %u position vertices!", vertices_size);
}

void Importer::Meshes::Utilities::GetNormals(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{
	if (!ai_mesh->HasNormals())
	{
		LOG("[ERROR] Imported Mesh has no normals!");
		return;
	}

	uint normals_size = ai_mesh->mNumVertices * 3;													// There will be 3 normal coordinates per vertex.
	r_mesh->normals.resize(normals_size);															// Allocating in advance the memory required to store all the normals.

	memcpy(&r_mesh->normals[0], ai_mesh->mNormals, sizeof(float) * normals_size);

	LOG("[STATUS] Imported %u normals!", normals_size);
}

void Importer::Meshes::Utilities::GetTexCoords(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{	
	if (!ai_mesh->HasTextureCoords(0))
	{
		LOG("[ERROR] Imported Mesh has no tex coords!");
		return;
	}

	uint tex_coords_size = ai_mesh->mNumVertices * 2;												// There will be 2 tex coordinates per vertex.
	r_mesh->tex_coords.resize(tex_coords_size);														// Allocating in advance the memory required to store all the texture coordinates.

	for (uint i = 0; i < ai_mesh->mNumVertices; ++i)
	{
		r_mesh->tex_coords[i * 2]		= ai_mesh->mTextureCoords[0][i].x;
		r_mesh->tex_coords[i * 2 + 1]	= ai_mesh->mTextureCoords[0][i].y;
	}

	LOG("[STATUS] Imported %u texture coordinates!", tex_coords_size);
}

void Importer::Meshes::Utilities::GetIndices(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{
	if (!ai_mesh->HasFaces())																		// Double checked if for whatever reason Generate Mesh is called independently.
	{
		LOG("[ERROR] Imported Mesh has no faces!");
		return;
	}

	uint indices_size = ai_mesh->mNumFaces * 3;														// The size of the indices vector will be equal to the amount of faces times 3 (triangles).
	r_mesh->indices.resize(indices_size);															// Allocating in advance the memory required to store all the indices.

	for (uint i = 0; i < ai_mesh->mNumFaces; ++i)
	{
		aiFace face = ai_mesh->mFaces[i];															// Getting the face that is currently being iterated.
		uint num_ind = face.mNumIndices;

		if (num_ind == 3)
		{
			memcpy(&r_mesh->indices[i * num_ind], face.mIndices, sizeof(uint) * num_ind);			// As each face has 3 elements (vertices), the memcpy will be done 3 vertices at a time.
		}
		else
		{
			LOG("[WARNING] Geometry face %u with != 3 indices!", i);
		}
	}

	LOG("[STATUS] Imported %u mesh indices!", indices_size);
}

void Importer::Meshes::Utilities::GetBones(const aiMesh* ai_mesh, R_Mesh* r_mesh)
{
	if (!ai_mesh->HasBones())
	{
		return;
	}
	
	for (uint i = 0; i < ai_mesh->mNumBones; ++i)
	{
		aiBone* ai_bone = ai_mesh->mBones[i];

		std::string name = ai_bone->mName.C_Str();
		
		float4x4 offset_matrix = float4x4::identity;
		Utilities::GetOffsetMatrix(ai_bone, offset_matrix);

		std::vector<VertexWeight> weights;
		Utilities::GetWeights(ai_bone, weights);

		r_mesh->bones.push_back(Bone(name, offset_matrix, weights));
	}
}

void Importer::Meshes::Utilities::GetOffsetMatrix(const aiBone* ai_bone, float4x4& offset_matrix)
{
	aiTransform ai_t;
	Transform	ma_t;

	ai_bone->mOffsetMatrix.Decompose(ai_t.position, ai_t.rotation, ai_t.scale);

	ma_t.position	= { ai_t.position.x, ai_t.position.y, ai_t.position.z };
	ma_t.rotation	= { ai_t.rotation.x, ai_t.rotation.y, ai_t.rotation.z, ai_t.rotation.w };
	ma_t.scale		= { ai_t.scale.x, ai_t.scale.y, ai_t.scale.z };

	offset_matrix = float4x4::FromTRS(ma_t.position, ma_t.rotation, ma_t.scale);
}

void Importer::Meshes::Utilities::GetWeights(const aiBone* ai_bone, std::vector<VertexWeight>& weights)
{
	for (uint j = 0; j < ai_bone->mNumWeights; ++j)
	{
		const aiVertexWeight& ai_weight = ai_bone->mWeights[j];

		uint vertex_id	= ai_weight.mVertexId;
		float weight	= ai_weight.mWeight;

		weights.push_back(VertexWeight(vertex_id, weight));
	}
}

uint Importer::Meshes::Utilities::GetBonesDataSize(const R_Mesh* r_mesh)
{
	if (r_mesh == nullptr)
	{
		LOG("[ERROR] Mesh Importer: Could not Get Bone Data Size! Error: Given R_Mesh* was nullptr.");
		return 0;
	}
	
	uint bones_size = 0;

	for (auto bone = r_mesh->bones.cbegin(); bone != r_mesh->bones.cend(); ++bone)
	{
		uint bone_size = 0;

		bone_size += (bone->name.length() * sizeof(char)) + sizeof(uint);											// Size of the name + size of the size variable.
		bone_size += 16 * sizeof(float);																			// Any given float4x4 is composed by 16 floats.
		bone_size += (bone->weights.size() * sizeof(VertexWeight)) + sizeof(uint);									// Size of each weight + size of the size variable.

		bones_size += bone_size;
	}

	return bones_size;
}

void Importer::Meshes::Utilities::StoreBoneName(const Bone& bone, char** cursor)
{
	uint bytes			= 0;
	uint name_length	= bone.name.length();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&name_length, bytes);
	*cursor += bytes;

	bytes = bone.name.length() * sizeof(char);
	memcpy(*cursor, (const void*)bone.name.c_str(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::StoreBoneOffsetMatrix(const Bone& bone, char** cursor)
{
	uint bytes = 16 * sizeof(float);
	memcpy(*cursor, (const void*)bone.offset_matrix.ptr(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::StoreBoneWeights(const Bone& bone, char** cursor)
{
	uint bytes			= 0;
	uint weights_size	= bone.weights.size();
	
	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&weights_size, bytes);
	*cursor += bytes;

	bytes = bone.weights.size() * sizeof(VertexWeight);
	memcpy(*cursor, (const void*)&bone.weights[0], bytes);
	//memcpy(*cursor, (const void*)bone.weights.data(), bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneName(char** cursor, Bone& bone)
{
	uint bytes			= 0;
	uint name_length	= 0;

	bytes = sizeof(uint);
	memcpy(&name_length, *cursor, bytes);
	*cursor += bytes;

	bone.name.resize(name_length);
	bytes = name_length * sizeof(char);
	memcpy(&bone.name[0], *cursor, bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneOffsetMatrix(char** cursor, Bone& bone)
{
	uint bytes = 16 * sizeof(float);
	memcpy(bone.offset_matrix.ptr(), *cursor, bytes);
	*cursor += bytes;
}

void Importer::Meshes::Utilities::LoadBoneWeights(char** cursor, Bone& bone)
{
	uint bytes			= 0;
	uint num_weights	= 0;

	bytes = sizeof(uint);
	memcpy(&num_weights, *cursor, bytes);
	*cursor += bytes;

	bone.weights.resize(num_weights);
	bytes = num_weights * sizeof(VertexWeight);
	memcpy(&bone.weights[0], *cursor, bytes);
	*cursor += bytes;
}

uint Importer::Meshes::Save(const R_Mesh* r_mesh, char** buffer)
{
	if (r_mesh == nullptr)
	{
		LOG("[ERROR] Could not Save Mesh to Library! Error: R_Mesh* was nullptr");
		return 0;
	}
	
	uint written = 0;

	uint header_data[HEADER_SIZE] = {
		r_mesh->vertices.size(),																	// 0 --> Num Vertices
		r_mesh->normals.size(), 																	// 1 --> Num Normals
		r_mesh->tex_coords.size(), 																	// 2 --> Num Texture Coordinates
		r_mesh->indices.size(), 																	// 3 --> Num Indices
		r_mesh->bones.size()																		// 4 --> Num Bones
	};

	uint header_data_size			= sizeof(header_data) + sizeof(uint);
	uint array_data_size			= (header_data[0] + header_data[1] + header_data[2]) * sizeof(float) + header_data[3] * sizeof(uint);
	uint bones_data_size			= Utilities::GetBonesDataSize(r_mesh);
	uint precalculated_data_size	= r_mesh->aabb.NumVertices() * sizeof(float) * 3;

	uint size = header_data_size + array_data_size + bones_data_size + precalculated_data_size;

	if (size == 0)
	{
		LOG("[WARNING] Mesh {%s} with UID [%u] had no data to Save!", r_mesh->GetAssetsFile(), r_mesh->GetUID());
		return 0;
	}

	*buffer				= new char[size];
	char* cursor		= *buffer;
	uint bytes			= 0;

	// --- HEADER DATA ---
	bytes = sizeof(header_data);
	memcpy_s(cursor, size, header_data, bytes);
	cursor += bytes;

	// --- VERTEX ARRAY DATA ---
	bytes = r_mesh->vertices.size() * sizeof(float);
	memcpy_s(cursor, size, &r_mesh->vertices[0], bytes);
	cursor += bytes;

	bytes = r_mesh->normals.size() * sizeof(float);
	memcpy_s(cursor, size, &r_mesh->normals[0], bytes);
	cursor += bytes;

	bytes = r_mesh->tex_coords.size() * sizeof(float);
	memcpy_s(cursor, size, &r_mesh->tex_coords[0], bytes);
	cursor += bytes;

	bytes = r_mesh->indices.size() * sizeof(uint);
	memcpy_s(cursor, size, &r_mesh->indices[0], bytes);
	cursor += bytes;

	// --- BONE DATA ---
	for (auto bone = r_mesh->bones.cbegin(); bone != r_mesh->bones.cend(); ++bone)
	{
		Utilities::StoreBoneName((*bone), &cursor);
		Utilities::StoreBoneOffsetMatrix((*bone), &cursor);
		Utilities::StoreBoneWeights((*bone), &cursor);
	}

	// --- PRECALCULATED DATA ---
	math::vec* aabb_corners = new math::vec[8];
	r_mesh->aabb.GetCornerPoints(aabb_corners);

	bytes = r_mesh->aabb.NumVertices() * sizeof(float) * 3;
	memcpy_s(cursor, size, aabb_corners, bytes);
	cursor += bytes;

	delete[] aabb_corners;

	// --- SAVING THE BUFFER ---
	std::string path	= MESHES_PATH + std::to_string(r_mesh->GetUID()) + MESHES_EXTENSION;
	written				= App->file_system->Save(path.c_str(), *buffer, size);
	if (written > 0)
	{	
		LOG("[STATUS] Importer: Successfully Saved Mesh { %s } to Library! Path: %s", r_mesh->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Meshes Importer: Could not Save { %s } to Library! Error: File System could not Write File.", r_mesh->GetAssetsFile());
	}

	return written;
}

bool Importer::Meshes::Load(const char* buffer, R_Mesh* r_mesh)
{
	bool ret = true;

	if (r_mesh == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Mesh from Library! Error: R_Mesh* was nullptr.");
		return false;
	}
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Mesh { %s } from Library! Error: Buffer was nullptr.", r_mesh->GetAssetsFile());
		return false;
	}
	
	char* cursor	= (char*)buffer;
	uint bytes		= 0;

	// --- HEADER DATA ---
	uint header_data[HEADER_SIZE];

	bytes = sizeof(header_data);
	memcpy_s(header_data, bytes, cursor, bytes);
	cursor += bytes;

	// --- VERTEX ARRAY DATA ---
	r_mesh->vertices.resize(header_data[0]);
	bytes = header_data[0] * sizeof(float);
	memcpy_s(&r_mesh->vertices[0], bytes, cursor, bytes);
	cursor += bytes;

	r_mesh->normals.resize(header_data[1]);
	bytes = header_data[1] * sizeof(float);
	memcpy_s(&r_mesh->normals[0], bytes, cursor, bytes);
	cursor += bytes;

	r_mesh->tex_coords.resize(header_data[2]);
	bytes = header_data[2] * sizeof(float);
	memcpy_s(&r_mesh->tex_coords[0], bytes, cursor, bytes);
	cursor += bytes;

	r_mesh->indices.resize(header_data[3]);
	bytes = header_data[3] * sizeof(uint);
	memcpy_s(&r_mesh->indices[0], bytes, cursor, bytes);
	cursor += bytes;

	// ---BONE DATA ---
	r_mesh->bones.resize(header_data[4]);
	for (uint i = 0; i < r_mesh->bones.size(); ++i)
	{
		Bone bone = Bone();

		Utilities::LoadBoneName(&cursor, bone);
		Utilities::LoadBoneOffsetMatrix(&cursor, bone);
		Utilities::LoadBoneWeights(&cursor, bone);

		r_mesh->bones[i] = bone;
	}

	// --- PRECALCULATED DATA ---
	float3 aabb_corners[8];

	bytes = sizeof(aabb_corners);
	memcpy_s(aabb_corners, bytes, cursor, bytes);
	cursor += bytes;

	r_mesh->aabb = AABB(aabb_corners[0], aabb_corners[7]);
	r_mesh->LoadBuffers();																									// STORING VBO, NBO, TBO, IBO DATA IS IRRELEVANT IN THIS CASE

	LOG("[STATUS] Importer: Successfully Loaded Mesh { %s } from Library!", r_mesh->GetAssetsFile());

	return ret;
}