#include "JSONParser.h"

#include "VariableTypedefs.h"
#include "FileSystemDefinitions.h"

#include "Random.h"

#include "Resource.h"

#include "MemoryManager.h"

Resource::Resource(ResourceType type) : 
type			(type),
uid				(Random::LCG::GetRandomUint()),
references		(0),
assetsPath		("[NONE]"), 
assetsFile		("[NONE]"), 
libraryPath		("[NONE]"), 
libraryFile		("[NONE]")
{

}

Resource::~Resource()
{

}

bool Resource::CleanUp()
{
	bool ret = true;

	assetsPath.clear();
	assetsFile.clear();
	libraryPath.clear();
	libraryFile.clear();

	assetsPath.shrink_to_fit();
	assetsFile.shrink_to_fit();
	libraryPath.shrink_to_fit();
	libraryFile.shrink_to_fit();

	return ret;
}

bool Resource::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	return ret;
}

bool Resource::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;

	return ret;
}

// --- RESOURCE METHODS ---
ResourceType Resource::GetType() const
{
	return type;
}

const char* Resource::GetTypeAsString() const
{
	switch (type)
	{
	case ResourceType::MODEL:		{ return "MODEL"; }		break;
	case ResourceType::MESH:		{ return "MESH"; }		break;
	case ResourceType::MATERIAL:	{ return "MATERIAL"; }	break;
	case ResourceType::TEXTURE:		{ return "TEXTURE"; }	break;
	case ResourceType::ANIMATION:	{ return "ANIMATION"; }	break;
	case ResourceType::SHADER:		{ return "SHADER"; }	break;
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
	return assetsPath.c_str();
}
const char* Resource::GetAssetsFile() const
{
	return assetsFile.c_str();
}
const char* Resource::GetLibraryPath() const
{
	return libraryPath.c_str();
}
const char* Resource::GetLibraryFile() const
{
	return libraryFile.c_str();
}

void Resource::SetAssetsPath(const char* assetsPath)
{
	this->assetsPath = assetsPath;
}

void Resource::SetAssetsFile(const char* assetsFile)
{
	this->assetsFile	= assetsFile;
}

void Resource::SetLibraryPath(const char* libraryPath)
{
	this->libraryPath = libraryPath;
}

void Resource::SetLibraryFile(const char* libraryFile)
{
	this->libraryFile = libraryFile;
}

void Resource::SetLibraryPathAndFile()
{
	std::string directory	= "";
	std::string file		= std::to_string(uid);
	std::string extension	= "";
	
	switch (type)
	{
	case ResourceType::ANIMATION:
		directory = ANIMATIONS_PATH;
		extension = ANIMATIONS_EXTENSION;
		break;

	case ResourceType::FOLDER:
		directory = FOLDERS_PATH;
		extension =	FOLDERS_EXTENSION;
		break;

	case ResourceType::MODEL:
		directory = MODELS_PATH;
		extension = MODELS_EXTENSION;
		break;

	case ResourceType::MESH:
		directory = MESHES_PATH;
		extension = MESHES_EXTENSION;
		break;

	case ResourceType::MATERIAL:
		directory = MATERIALS_PATH;
		extension = MATERIALS_EXTENSION;
		break;

	case ResourceType::TEXTURE:
		directory = TEXTURES_PATH;
		extension = TEXTURES_EXTENSION;
		break;

	case ResourceType::SCENE:
		directory = SCENES_PATH;
		extension = SCENES_EXTENSION;
		break;

	case ResourceType::SHADER:
		directory = SHADERS_PATH;
		extension = SHADERS_EXTENSION;
		break;
	}
	
	libraryPath = directory + file + extension;
	libraryFile = file + extension;
}