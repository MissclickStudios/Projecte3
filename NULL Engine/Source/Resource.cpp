#include "JSONParser.h"

#include "VariableTypedefs.h"
#include "FileSystemDefinitions.h"

#include "Random.h"

#include "Resource.h"

Resource::Resource(RESOURCE_TYPE type) : 
type			(type),
uid				(Random::LCG::GetRandomUint()),
references		(0),
assets_path		("[NONE]"), 
assets_file		("[NONE]"), 
library_path	("[NONE]"), 
library_file	("[NONE]")
{

}

Resource::~Resource()
{

}

bool Resource::CleanUp()
{
	bool ret = true;

	assets_path.clear();
	assets_file.clear();
	library_path.clear();
	library_file.clear();

	assets_path.shrink_to_fit();
	assets_file.shrink_to_fit();
	library_path.shrink_to_fit();
	library_file.shrink_to_fit();

	return ret;
}

bool Resource::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	return ret;
}

bool Resource::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;

	return ret;
}

// --- RESOURCE METHODS ---
RESOURCE_TYPE Resource::GetType() const
{
	return type;
}

const char* Resource::GetTypeAsString() const
{
	switch (type)
	{
	case RESOURCE_TYPE::MESH:		{ return "MESH"; }		break;
	case RESOURCE_TYPE::MATERIAL:	{ return "MATERIAL"; }	break;
	case RESOURCE_TYPE::TEXTURE:	{ return "TEXTURE"; }	break;
	}

	return "NONE";
}

uint32 Resource::GetUID() const
{
	return uid;
}

void Resource::ForceUID(const uint32& UID)
{
	uid = UID;											// TMP
	
	//uid = Random::LCG::GetRandomUint();
}

uint Resource::GetReferences() const
{
	return references;
}

void Resource::SetReferences(const uint& references)
{
	this->references = references;
}

const char* Resource::GetAssetsPath() const
{
	return assets_path.c_str();
}
const char* Resource::GetAssetsFile() const
{
	return assets_file.c_str();
}
const char* Resource::GetLibraryPath() const
{
	return library_path.c_str();
}
const char* Resource::GetLibraryFile() const
{
	return library_file.c_str();
}

void Resource::SetAssetsPath(const char* assets_path)
{
	this->assets_path = assets_path;
}

void Resource::SetAssetsFile(const char* assets_file)
{
	this->assets_file	= assets_file;
}

void Resource::SetLibraryPath(const char* library_path)
{
	this->library_path = library_path;
}

void Resource::SetLibraryFile(const char* library_file)
{
	this->library_file = library_file;
}

void Resource::SetLibraryPathAndFile()
{
	std::string directory	= "";
	std::string file		= std::to_string(uid);
	std::string extension	= "";
	
	switch (type)
	{
	case RESOURCE_TYPE::ANIMATION:
		directory = ANIMATIONS_PATH;
		extension = ANIMATIONS_EXTENSION;
		break;

	case RESOURCE_TYPE::FOLDER:
		directory = FOLDERS_PATH;
		extension =	FOLDERS_EXTENSION;
		break;

	case RESOURCE_TYPE::MODEL:
		directory = MODELS_PATH;
		extension = MODELS_EXTENSION;
		break;

	case RESOURCE_TYPE::MESH:
		directory = MESHES_PATH;
		extension = MESHES_EXTENSION;
		break;

	case RESOURCE_TYPE::MATERIAL:
		directory = MATERIALS_PATH;
		extension = MATERIALS_EXTENSION;
		break;

	case RESOURCE_TYPE::TEXTURE:
		directory = TEXTURES_PATH;
		extension = TEXTURES_EXTENSION;
		break;

	case RESOURCE_TYPE::SCENE:
		directory = SCENES_PATH;
		extension = SCENES_EXTENSION;
		break;
	}
	
	library_path = directory + file + extension;
	library_file = file + extension;
}