// ----------------------------------------------------
// I_Materials.cpp --- Importing materials from files.
// Interface class between the engine and Assimp.
// ----------------------------------------------------

#include "OpenGL.h"
#include "Assimp.h"

#include "VariableTypedefs.h"
#include "Log.h"
#include "Color.h"

#include "Application.h"
#include "M_FileSystem.h"
#include "R_Material.h"
#include "R_Texture.h"

#include "I_Textures.h"

#include "I_Materials.h"

#include "MemoryManager.h"

using namespace Importer::Materials;																	// Not a good thing to do but it will be employed sparsely and only inside this .cpp

void Importer::Materials::Import(const aiMaterial* assimpMaterial, R_Material* rMaterial)
{
	if (rMaterial == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Material! Error: R_Material* was nullptr.");
		return;
	}
	if (assimpMaterial == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Material { %s }! Error: aiMaterial* was nullptr.", rMaterial->GetAssetsFile());
		return;
	}
	
	std::string dirPath	= ASSETS_TEXTURES_PATH + App->file_system->GetLastDirectory(rMaterial->GetAssetsPath());			// Dirty setting of the assets path.
	std::string file		= "";
	std::string fullPath	= "";
	
	aiColor4D color;
	if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)															// Could also get specular and ambient occlusion colours.
	{
		rMaterial->diffuse_color = Color(color.r, color.g, color.b, color.a);
	}

	aiString texPath;
	if (assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)												// At the moment only DIFFUSE textures will be imported.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::DIFFUSE, 0, fullPath));
	}
	if (assimpMaterial->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)											// Checking if there is a SPECULAR texture.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::SPECULAR, 0, fullPath));
	}
	if (assimpMaterial->GetTexture(aiTextureType_AMBIENT, 0, &texPath) == AI_SUCCESS)												// Checking if there is a AMBIENT texture.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::AMBIENT, 0, fullPath));
	}
	if (assimpMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &texPath) == AI_SUCCESS)											// Checking if there is a EMISSIVE texture.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::EMISSIVE, 0, fullPath));
	}
	if (assimpMaterial->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS)												// Checking if there is a HEIGHT texture.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::HEIGHT, 0, fullPath));
	}
	if (assimpMaterial->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)												// Checking if there is a NORMALS texture.
	{
		file		= App->file_system->GetFileAndExtension(texPath.C_Str());
		fullPath	= dirPath + file;
		rMaterial->materials.push_back(MaterialData(TEXTURE_TYPE::NORMALS, 0, fullPath));
	}
}

uint Importer::Materials::Save(const R_Material* rMaterial, char** buffer)
{
	uint written = 0;

	if (rMaterial == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save Material to Library! Error: R_Material* was nullptr.");
		return 0;
	}
	
	float color[4] = {
		rMaterial->diffuse_color.r,
		rMaterial->diffuse_color.g,
		rMaterial->diffuse_color.b,
		rMaterial->diffuse_color.a,
	};

	uint size = sizeof(color);

	if (size == 0)
	{
		return 0;
	}

	*buffer			= new char[size];
	char* cursor	= *buffer;
	uint bytes		= 0;

	// --- COLOR DATA ---
	bytes = sizeof(color);
	memcpy_s(cursor, size, color, bytes);
	cursor += bytes;

	// --- SAVING THE BUFFER ---
	std::string path = std::string(MATERIALS_PATH) + std::to_string(rMaterial->GetUID()) + std::string(MATERIALS_EXTENSION);

	written = App->file_system->Save(path.c_str(), *buffer, size);
	if (written > 0)
	{
		LOG("[STATUS] Importer Materials: Successfully Saved Material { %s } to Library! Path: { %s }", rMaterial->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Importer: Could not Save Material { %s } to Library! Error: File System could not Write File.", rMaterial->GetAssetsFile());
	}

	return written;
}

bool Importer::Materials::Load(const char* buffer, R_Material* rMaterial)
{
	bool ret = true;
	
	if (rMaterial == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Material from Library! Error: R_Material* was nullptr.");
		return false;
	}
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Material { %s } from Library! Error: Buffer was nullptr.", rMaterial->GetAssetsFile());
		return false;
	}

	char* cursor = (char*)buffer;
	uint bytes	 = 0;
	
	float color[4];
	
	bytes	= sizeof(color);
	memcpy_s(color, bytes, cursor, bytes);
	cursor	+= bytes;

	rMaterial->diffuse_color.Set(color[0], color[1], color[2], color[3]);

	LOG("[STATUS] Importer: Successfully Loaded Material { %s } from Library!", rMaterial->GetAssetsFile());

	return ret;
}