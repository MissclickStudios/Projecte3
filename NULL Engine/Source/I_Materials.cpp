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

void Importer::Materials::Import(const aiMaterial* ai_material, R_Material* r_material)
{
	if (r_material == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Material! Error: R_Material* was nullptr.");
		return;
	}
	if (ai_material == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Material { %s }! Error: aiMaterial* was nullptr.", r_material->GetAssetsFile());
		return;
	}
	
	std::string dir_path	= ASSETS_TEXTURES_PATH + App->file_system->GetLastDirectory(r_material->GetAssetsPath());			// Dirty setting of the assets path.
	std::string file		= "";
	std::string full_path	= "";
	
	aiColor4D color;
	if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)															// Could also get specular and ambient occlusion colours.
	{
		r_material->diffuse_color = Color(color.r, color.g, color.b, color.a);
	}

	aiString tex_path;
	if (ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS)												// At the moment only DIFFUSE textures will be imported.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::DIFFUSE, 0, full_path));
	}
	if (ai_material->GetTexture(aiTextureType_SPECULAR, 0, &tex_path) == AI_SUCCESS)											// Checking if there is a SPECULAR texture.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::SPECULAR, 0, full_path));
	}
	if (ai_material->GetTexture(aiTextureType_AMBIENT, 0, &tex_path) == AI_SUCCESS)												// Checking if there is a AMBIENT texture.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::AMBIENT, 0, full_path));
	}
	if (ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &tex_path) == AI_SUCCESS)											// Checking if there is a EMISSIVE texture.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::EMISSIVE, 0, full_path));
	}
	if (ai_material->GetTexture(aiTextureType_HEIGHT, 0, &tex_path) == AI_SUCCESS)												// Checking if there is a HEIGHT texture.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::HEIGHT, 0, full_path));
	}
	if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS)												// Checking if there is a NORMALS texture.
	{
		file		= App->file_system->GetFileAndExtension(tex_path.C_Str());
		full_path	= dir_path + file;
		r_material->materials.push_back(MaterialData(TEXTURE_TYPE::NORMALS, 0, full_path));
	}
}

uint Importer::Materials::Save(const R_Material* r_material, char** buffer)
{
	uint written = 0;

	if (r_material == nullptr)
	{
		LOG("[ERROR] Importer: Could not Save Material to Library! Error: R_Material* was nullptr.");
		return 0;
	}
	
	float color[4] = {
		r_material->diffuse_color.r,
		r_material->diffuse_color.g,
		r_material->diffuse_color.b,
		r_material->diffuse_color.a,
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
	std::string path = std::string(MATERIALS_PATH) + std::to_string(r_material->GetUID()) + std::string(MATERIALS_EXTENSION);

	written = App->file_system->Save(path.c_str(), *buffer, size);
	if (written > 0)
	{
		LOG("[STATUS] Importer Materials: Successfully Saved Material { %s } to Library! Path: { %s }", r_material->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Importer: Could not Save Material { %s } to Library! Error: File System could not Write File.", r_material->GetAssetsFile());
	}

	return written;
}

bool Importer::Materials::Load(const char* buffer, R_Material* r_material)
{
	bool ret = true;
	
	if (r_material == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Material from Library! Error: R_Material* was nullptr.");
		return false;
	}
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Material { %s } from Library! Error: Buffer was nullptr.", r_material->GetAssetsFile());
		return false;
	}

	char* cursor = (char*)buffer;
	uint bytes	 = 0;
	
	float color[4];
	
	bytes	= sizeof(color);
	memcpy_s(color, bytes, cursor, bytes);
	cursor	+= bytes;

	r_material->diffuse_color.Set(color[0], color[1], color[2], color[3]);

	LOG("[STATUS] Importer: Successfully Loaded Material { %s } from Library!", r_material->GetAssetsFile());

	return ret;
}