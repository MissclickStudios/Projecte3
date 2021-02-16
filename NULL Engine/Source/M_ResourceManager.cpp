#include "VariableTypedefs.h"
#include "JSONParser.h"

#include "Time.h"

#include "Importer.h"
#include "I_Scenes.h"
#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Textures.h"
#include "I_Folders.h"
#include "I_Animations.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "Resource.h"
#include "R_Mesh.h"
#include "R_Material.h"
#include "R_Texture.h"
#include "R_Model.h"
#include "R_Folder.h"
#include "R_Scene.h"
#include "R_Animation.h"

#include "M_ResourceManager.h"

#include "MemoryManager.h"

typedef std::pair<std::map<uint32, Resource*>::iterator, bool>	INSERT_RESULT;													// Also valid for emplace oparations.
typedef std::map<uint32, Resource*>::iterator					RESOURCE_ITEM;
typedef std::map<uint32, std::string>::iterator					LIBRARY_ITEM;
typedef std::map<std::string, uint32>::iterator					FILE_ITEM;

M_ResourceManager::M_ResourceManager() : Module("ResourceManager"),
file_refresh_timer	(0.0f),
file_refresh_rate	(0.0f)
{

}

M_ResourceManager::~M_ResourceManager()
{

}
 
bool M_ResourceManager::Init(ParsonNode& configuration)
{
	bool ret = true;

	//file_refresh_rate = (float)configuration.GetNumber("RefreshRate");
	file_refresh_rate = 5.0f;

	return ret;
}

bool M_ResourceManager::Start()
{
	bool ret = true;

	RefreshDirectoryFiles(ASSETS_DIRECTORY);
	RefreshDirectoryFiles(ENGINE_DIRECTORY);

	return ret;
}

UPDATE_STATUS M_ResourceManager::PreUpdate(float dt)
{
	UPDATE_STATUS status = UPDATE_STATUS::CONTINUE;

	file_refresh_timer += Time::Real::GetDT();

	if (file_refresh_timer > file_refresh_rate)
	{
		RESOURCE_ITEM item = resources.begin();
		while (item != resources.end())
		{
			if (item->second->GetReferences() == 0)															// Clear all Reference 0 resources that might have gone past the 
			{																								// FreeResource() method.
				uint32 resource_uid = item->second->GetUID();												// 
				++item;																						// Setting item to the next element so the reference is not lost after
				DeallocateResource(resource_uid);															// erasing the element with the resource_uid from the resources std::map.
				continue;																					// Going to the next iteration so item is not updated twice in the same loop.
			}

			++item;
		}
		
		//RefreshDirectoryFiles(ASSETS_DIRECTORY);

		file_refresh_timer = 0.0f;
	}

	return status;
}

UPDATE_STATUS M_ResourceManager::Update(float dt)
{
	UPDATE_STATUS status = UPDATE_STATUS::CONTINUE;

	

	return status;
}

UPDATE_STATUS M_ResourceManager::PostUpdate(float dt)
{
	UPDATE_STATUS status = UPDATE_STATUS::CONTINUE;



	return status;
}

bool M_ResourceManager::CleanUp()
{
	bool ret = true;

	std::map<uint32, Resource*>::iterator r_item;
	for (r_item = resources.begin(); r_item != resources.end(); ++r_item)
	{
		r_item->second->CleanUp();
		RELEASE(r_item->second);
	}

	resources.clear();
	library.clear();

	return ret;
}

bool M_ResourceManager::SaveConfiguration(ParsonNode& configuration) const
{
	bool ret = true;



	return ret;
}

bool M_ResourceManager::LoadConfiguration(ParsonNode& configuration)
{
	bool ret = true;



	return ret;
}

// ----- M_RESOURCEMANAGER METHODS -----
// --- ASSETS MONITORING METHODS ---
void M_ResourceManager::RefreshDirectoryFiles(const char* directory)
{
	if (directory == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Refresh Directory Files! Error: Directory string was nullptr.");
		return;
	}
	
	std::vector<std::string> files_to_import;															// Refresh folders and create .meta files for all those files that do not have one.
	std::vector<std::string> files_to_update;															// Also update the .meta and reimport all those files that have been modified.
	std::vector<std::string> files_to_delete;

	RefreshDirectory(directory, files_to_import, files_to_update, files_to_delete);

	for (uint i = 0; i < files_to_delete.size(); ++i)
	{
		//DeleteFromAssets(files_to_delete[i].c_str());
		DeleteFromLibrary(files_to_delete[i].c_str());
	}
	for (uint i = 0; i < files_to_update.size(); ++i)
	{
		DeleteFromLibrary(files_to_update[i].c_str());
		ImportFile(files_to_update[i].c_str());
	}
	for (uint i = 0; i < files_to_import.size(); ++i)
	{
		ImportFile(files_to_import[i].c_str());
	}

	files_to_delete.clear();
	files_to_update.clear();
	files_to_import.clear();
}

void M_ResourceManager::RefreshDirectory(const char* directory, std::vector<std::string>& files_to_import, 
												std::vector<std::string>& files_to_update, std::vector<std::string>& files_to_delete)
{
	if (directory == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Refresh Directory! Error: Directory string was nullptr.");
		return;
	}
	
	std::vector<std::string> directories;
	std::vector<std::string> asset_files;
	std::vector<std::string> meta_files;
	std::map<std::string, std::string> file_pairs;

	App->file_system->DiscoverAllFiles(directory, asset_files, directories, DOTLESS_META_EXTENSION);				// Directories (folders) will be ignored for now.
	App->file_system->GetAllFilesWithExtension(directory, DOTLESS_META_EXTENSION, meta_files);
	
	FindFilesToImport(asset_files, meta_files, file_pairs, files_to_import);										// Always call in this order!
	FindFilesToUpdate(file_pairs, files_to_update);																	// At the very least FindFilesToImport() has to be the first to be called
	FindFilesToDelete(meta_files, file_pairs, files_to_delete);														// as it is the one to fill file_pairs with asset and meta files!

	LoadValidFilesIntoLibrary(file_pairs);																			// Will emplace all valid files' UID & library path into the library map.

	file_pairs.clear();
	meta_files.clear();
	asset_files.clear();
	directories.clear();
}

void M_ResourceManager::FindFilesToImport(const std::vector<std::string>& asset_files, const std::vector<std::string>& meta_files, 
											std::map<std::string, std::string>& file_pairs, std::vector<std::string>& files_to_import)
{
	std::map<std::string, uint> meta_tmp;
	for (uint i = 0; i < meta_files.size(); ++i)																	// Adding the meta_files to a map so the elements can be found with ease.
	{
		meta_tmp.emplace(meta_files[i], i);
	}
	
	std::string meta_file = "[NONE]";
	std::map<std::string, uint>::iterator item;
	for (uint i = 0; i < asset_files.size(); ++i)																	// Assets files whose meta file is missing will be imported.
	{
		meta_file	= asset_files[i] + META_EXTENSION;
		item		= meta_tmp.find(meta_file);

		if (item != meta_tmp.end() /*&& MetaFileIsValid(asset_files[i].c_str())*/)
		{
			file_pairs.emplace(asset_files[i], item->first);

			if (!MetaFileIsValid(asset_files[i].c_str()))															// In case the pair exists but the meta file is outdated.
			{
				files_to_import.push_back(asset_files[i]);
			}
		}
		else
		{
			files_to_import.push_back(asset_files[i]);
		}
	}

	meta_tmp.clear();
}

void M_ResourceManager::FindFilesToUpdate(const std::map<std::string, std::string>& file_pairs, std::vector<std::string>& files_to_update)
{
	uint64 asset_mod_time	= 0;
	uint64 meta_mod_time	= 0;
	std::map<std::string, std::string>::const_iterator item;
	for (item = file_pairs.begin(); item != file_pairs.end(); ++item)
	{
		asset_mod_time	= App->file_system->GetLastModTime(item->first.c_str());						// Files with different modification time than their meta files
		meta_mod_time	= GetAssetFileModTimeFromMeta(item->first.c_str());								// will need to be updated (Delete prev + Import new).

		if (asset_mod_time != meta_mod_time)
		{
			files_to_update.push_back(item->first);
		}
	}
}

void M_ResourceManager::FindFilesToDelete(const std::vector<std::string>& meta_files, const std::map<std::string, std::string>& file_pairs, std::vector<std::string>& files_to_delete)
{
	std::string assets_path = "[NONE]";
	for (uint i = 0; i < meta_files.size(); ++i)																	// Meta files whose asset_file is missing will be deleted.
	{
		assets_path = meta_files[i];
		assets_path.resize(assets_path.size() - std::string(META_EXTENSION).size());								// Dirty-but-quick way of eliminating the extension.

 		if (file_pairs.find(assets_path) == file_pairs.end())														// If cannot find assets_path in the paired files.
		{						
			files_to_delete.push_back(assets_path);
		}
	}
}

void M_ResourceManager::LoadValidFilesIntoLibrary(const std::map<std::string, std::string>& file_pairs)
{
	std::map<std::string, std::string>::const_iterator item;
	for (item = file_pairs.cbegin(); item != file_pairs.cend(); ++item)
	{
		LoadMetaLibraryPairsIntoLibrary(item->first.c_str());
	}
}

bool M_ResourceManager::DeleteFromAssets(const char* assets_path)
{
	bool ret = true;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Delete File from Assets! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::vector<uint32> resource_uids;
	std::vector<std::string> files_to_delete;

	GetResourceUIDsFromMeta(assets_path, resource_uids);
	GetLibraryFilePathsFromMeta(assets_path, files_to_delete);
	
	std::string meta_path = assets_path + std::string(META_EXTENSION);
	files_to_delete.push_back(assets_path);
	files_to_delete.push_back(meta_path);

	for (uint i = 0; i < resource_uids.size(); ++i)
	{
		DeleteResource(resource_uids[i]);
	}

	for (uint i = 0; i < files_to_delete.size(); ++i)
	{
		/*ret =*/ App->file_system->Remove(files_to_delete[i].c_str());
	}

	files_to_delete.clear();
	resource_uids.clear();

	return ret;
}

bool M_ResourceManager::DeleteFromLibrary(const char* assets_path)
{
	bool ret = true;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not delete Files from Library! Error: Given Assets Path was nullptr.");
		return false;
	}

	if (!HasMetaFile(assets_path))
	{
		LOG("[ERROR] Resource Manager: Could not delete { %s }'s Files from Library! Error: Given Assets Path had no associated Meta File.", assets_path);
		return false;
	}

	std::vector<uint32> resource_uids;
	std::vector<std::string> files_to_delete;

	GetResourceUIDsFromMeta(assets_path, resource_uids);
	GetLibraryFilePathsFromMeta(assets_path, files_to_delete);

	for (uint i = 0; i < resource_uids.size(); ++i)
	{
		DeleteResource(resource_uids[i]);
	}

	for (uint i = 0; i < files_to_delete.size(); ++i)
	{
		App->file_system->Remove(files_to_delete[i].c_str());
	}

	files_to_delete.clear();
	resource_uids.clear();

	return ret;
}

bool M_ResourceManager::GetResourceUIDsFromMeta(const char* assets_path, std::vector<uint32>& resource_UIDs)
{
	bool ret = true;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource UIDs from Meta! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string error_string = "[ERROR] Resource Manager: Could not get Resource UIDs from { " + std::string(assets_path) + " }'s Meta";

	char* buffer					= nullptr;
	ParsonNode meta_root			= LoadMetaFile(assets_path, &buffer);
	ParsonArray contained_array		= meta_root.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no correspondent Meta File.", error_string.c_str());
		return false;
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: Contained Array in Meta File was not valid.", error_string.c_str());
		return false;
	}

	// --- MAIN RESOURCE
	uint32 resource_uid = (uint32)meta_root.GetNumber("UID");
	if (resource_uid == 0)
	{
		LOG("%s! Error: Main Resource UID was 0.", error_string.c_str());
		return false;
	}

	resource_UIDs.push_back(resource_uid);

	// --- CONTAINED RESOURCES
	uint32 contained_uid		= 0;
	ParsonNode contained_node	= ParsonNode();
	for (uint i = 0; i < contained_array.size; ++i)
	{
		contained_node = contained_array.GetNode(i);
		if (!contained_node.NodeIsValid())
		{
			continue;
		}

		contained_uid = 0;																									// Resetting the contained uid
		contained_uid = (uint32)contained_node.GetNumber("UID");
		if (contained_uid == 0)
		{
			LOG("[WARNING] Resource Manager: Contained UID was not valid!");
			continue;
		}

		resource_UIDs.push_back(contained_uid);
	}

	return ret;
}

bool M_ResourceManager::GetLibraryFilePathsFromMeta(const char* assets_path, std::vector<std::string>& file_paths)
{
	bool ret = true;
	
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}
	
	std::string error_string = "[ERROR] Resoruce Manager: Could not get Library File Paths from { " + std::string(assets_path) + " }'s Meta File";

	char* buffer					= nullptr;
	ParsonNode meta_root			= LoadMetaFile(assets_path, &buffer);
	ParsonArray contained_array		= meta_root.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", error_string.c_str());
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: ContainedResources array in Meta File was not valid.", error_string.c_str());
		return false;
	}

	std::string directory = "[NONE]";
	std::string extension = "[NONE]";

	// --- MAIN RESOURCE
	uint32 resource_uid		= (uint32)meta_root.GetNumber("UID");
	RESOURCE_TYPE type		= (RESOURCE_TYPE)meta_root.GetNumber("Type");
	bool success			= GetLibraryDirectoryAndExtensionFromType(type, directory, extension);
	if (!success)
	{
		LOG("%s! Error: Could not get the Library Directory and Extension from Resource Type.", error_string.c_str());
		return false;
	}
	if (resource_uid == 0)
	{
		LOG("%s! Error: Main Resource UID was 0.", error_string.c_str());
		return false;
	}
	if (directory == "[NONE]" || extension == "[NONE]")
	{
		LOG("%s! Error: Main Resource Library Directory or Extension strings were not valid.", error_string.c_str());
		return false;
	}

	std::string library_path = directory + std::to_string(resource_uid) + extension;
	file_paths.push_back(library_path);

	// --- CONTAINED RESOURCES
	ParsonNode contained_node		= ParsonNode();
	uint32 contained_uid			= 0;
	RESOURCE_TYPE contained_type	= RESOURCE_TYPE::NONE;
	std::string contained_path		= "[NONE]";
	for (uint i = 0; i < contained_array.size; ++i)
	{
		contained_node = contained_array.GetNode(i);
		if (!contained_node.NodeIsValid())
		{
			continue;
		}

		directory = "[NONE]";
		extension = "[NONE]";

		contained_uid	= (uint32)contained_node.GetNumber("UID");
		contained_type	= (RESOURCE_TYPE)contained_node.GetNumber("Type");
		success			= GetLibraryDirectoryAndExtensionFromType(contained_type, directory, extension);
		if (!success)
		{
			continue;
		}
		if (contained_uid == 0)
		{
			continue;
		}
		if (directory == "[NONE]" || extension == "[NONE]")
		{
			continue;
		}

		contained_path = directory + std::to_string(contained_uid) + extension;
		file_paths.push_back(contained_path);
	}

	return ret;
}

bool M_ResourceManager::GetLibraryDirectoryAndExtensionFromType(const RESOURCE_TYPE& type, std::string& directory, std::string& extension)
{
	bool ret = true;
	
	switch (type)
	{
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
	case RESOURCE_TYPE::FOLDER:
		directory = FOLDERS_PATH;
		extension = FOLDERS_EXTENSION;
		break;
	case RESOURCE_TYPE::SCENE:
		directory = SCENES_PATH;
		extension = SCENES_EXTENSION;
		break;
	case RESOURCE_TYPE::ANIMATION:
		directory = ANIMATIONS_PATH;
		extension = ANIMATIONS_EXTENSION;
		break;
	case RESOURCE_TYPE::NONE:
		ret = false;
		break;
	}

	return ret;
}

bool M_ResourceManager::LoadMetaLibraryPairsIntoLibrary(const char* assets_path)
{
	bool ret = true;
	
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not load Meta Library Pairs into Library! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::map<uint32, std::string> library_pairs;
	GetLibraryPairsFromMeta(assets_path, library_pairs);

	std::map<uint32, std::string>::iterator item;
	for (item = library_pairs.begin(); item != library_pairs.end(); ++item)
	{
		library.emplace(item->first, item->second);
	}

	library_pairs.clear();

	return ret;
}

bool M_ResourceManager::GetLibraryPairsFromMeta(const char* assets_path, std::map<uint32, std::string>& pairs)
{
	bool ret = true;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library Pairs from File's associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string error_string = "[ERROR] Resource Manager: Could not get Libarary Pairs from { " + std::string(assets_path) + " }'s associated Meta File";

	std::vector<uint32> resource_uids;
	std::vector<std::string> library_paths;

	GetResourceUIDsFromMeta(assets_path, resource_uids);
	GetLibraryFilePathsFromMeta(assets_path, library_paths);

	if (resource_uids.size() != library_paths.size())
	{
		LOG("%s! Error: Mismatched amount of Resource UIDs and Library Paths.", error_string.c_str());
	}

	for (uint i = 0; i < resource_uids.size(); ++i)
	{
		pairs.emplace(resource_uids[i], library_paths[i]);
	}

	resource_uids.clear();
	library_paths.clear();

	return ret;
}

uint64 M_ResourceManager::GetAssetFileModTimeFromMeta(const char* assets_path)
{
	uint64 ret = 0;
	
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string error_string = "[ERROR] Resoruce Manager: Could not get Asset File Mod Time from { " + std::string(assets_path) + " }'s Meta File";

	char* buffer					= nullptr;
	ParsonNode meta_root			= LoadMetaFile(assets_path, &buffer);
	ParsonArray contained_array		= meta_root.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", error_string.c_str());
		return 0;
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: ContainedResources array in Meta File was not valid.", error_string.c_str());
		return 0;
	}

	ret = (uint64)meta_root.GetNumber("ModificationTime");

	return ret;
}

// --- IMPORT FILE METHODS--
uint32 M_ResourceManager::ImportFile(const char* assets_path)
{
	uint32 resource_uid = 0;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: Path was nullptr.");
		return 0;
	}

	if (App->file_system->GetFileExtension(assets_path) == "json" 
		|| App->file_system->GetFileExtension(assets_path) == "JSON")													// TMP until R_Scene has been fully implemented.
	{
		return 0;
	}

	assets_path			= GetValidPath(assets_path);
	bool meta_is_valid	= MetaFileIsValid(assets_path);
	if (!meta_is_valid)
	{
		if (HasMetaFile(assets_path))
		{
			DeleteFromLibrary(assets_path);																				// Cleaning any remaining Library files.
		}
		
		resource_uid = ImportFromAssets(assets_path); 

		if (resource_uid == 0)
		{
			LOG("[ERROR] Resource Manager: Could not Import File { %s }! Error: See [IMPORTER] and Resource Manager ERRORS.", assets_path);
			return 0;
		}
	}
	else
	{
		LOG("[WARNING] Resource Manager: The File to Import was already in the Library!");

		std::map<uint32, std::string> library_items;
		GetLibraryPairsFromMeta(assets_path, library_items);

		std::map<uint32, std::string>::iterator item;
		for (item = library_items.begin(); item != library_items.end(); ++item)
		{
			if (library.find(item->first) == library.end())
			{
				library.emplace(item->first, item->second);
			}
		}

		resource_uid = library_items.begin()->first;

		library_items.clear();
	}

	return resource_uid;
}

uint32 M_ResourceManager::ImportFromAssets(const char* assets_path)
{
	uint32 resource_uid = 0;
	
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: Path was nullptr.");
		return 0;
	}

	char* buffer = nullptr;
	uint read = App->file_system->Load(assets_path, &buffer);
	if (read > 0)
	{
		RESOURCE_TYPE type = GetTypeFromAssetsExtension(assets_path);
		Resource* resource = CreateResource(type, assets_path);

		bool success = false;
		switch (type)
		{
		case RESOURCE_TYPE::MODEL:		{ success = Importer::ImportScene(buffer, read, (R_Model*)resource); }		break;
		case RESOURCE_TYPE::MESH:		{ success = Importer::ImportMesh(buffer, (R_Mesh*)resource); }				break;
		case RESOURCE_TYPE::TEXTURE:	{ success = Importer::ImportTexture(buffer, read, (R_Texture*)resource); }	break;
		case RESOURCE_TYPE::SCENE:		{ /*success = HAVE A FUNCTIONAL R_SCENE AND LOAD/SAVE METHODS*/}			break;
		}

		RELEASE_ARRAY(buffer);

		if (!success)
		{
			LOG("[ERROR] Resource Manager: Could not Import File %s! Error: Check for [IMPORTER] errors in the Console Panel.", assets_path);
			DeallocateResource(resource);
			return 0;
		}

		resource_uid = resource->GetUID();
		SaveResourceToLibrary(resource);
		DeallocateResource(resource);
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not Import File %s! Error: File System could not Read the File.", assets_path);
		return 0;
	}

	return resource_uid;
}

uint32 M_ResourceManager::LoadFromLibrary(const char* assets_path)
{
	uint32 resource_uid = 0;
	
	std::string error_string = "[ERROR] Resource Manager: Could not Load File from the given Library Path";

	assets_path = GetValidPath(assets_path);

	if (assets_path == nullptr)
	{
		LOG("%s! Error: Path was nullptr.", error_string.c_str());
		return 0;
	}

	char* buffer			= nullptr;
	ParsonNode meta_root	= LoadMetaFile(assets_path, &buffer);
	RELEASE_ARRAY(buffer);

	bool meta_is_valid		= MetaFileIsValid(meta_root);
	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node.", error_string.c_str());
		return 0;
	}
	if (!meta_is_valid)
	{
		LOG("%s! Error: Could not Validate the Meta File.", error_string.c_str());
		return 0;
	}

	resource_uid					= (uint32)meta_root.GetNumber("UID");
	ParsonArray contained_array		= meta_root.GetArray("ContainedResources");
	
	if (resources.find(resource_uid) != resources.end())
	{
		return resource_uid;																									// If the File To Load's Resource is already in memory.
	}	

	Resource* result	= nullptr;
	result				= AllocateResource(resource_uid, assets_path);
	if (result == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Allocate Resource %lu in memory!", resource_uid);
		return 0;
	}

	std::string contained_path = "";
	std::string contained_name = "";
	for (uint i = 0; i < contained_array.size; ++i)
	{
		ParsonNode contained_node = contained_array.GetNode(i);

		App->file_system->SplitFilePath(assets_path, &contained_path, nullptr, nullptr);									// --- TMP Until Something Functional Is In Place.
		contained_name	= contained_node.GetString("Name");																	// 
		contained_path += contained_name;																					// -----------------------------------------------

		uint32 contained_uid = (uint32)contained_node.GetNumber("UID");

		if (resources.find(contained_uid) != resources.end())																// No need to allocate if it is already allocated.
		{
			continue;
		}

		result = AllocateResource(contained_uid, contained_path.c_str());
		if (result == nullptr)
		{
			LOG("[WARNING] Resource Manager: Could not allocate Contained Resource! UID: %lu, Name: %s", contained_uid, contained_name.c_str());
		}

		contained_name.clear();
		contained_path.clear();
	}

	return resource_uid;
}

uint M_ResourceManager::SaveResourceToLibrary(Resource* resource)
{	
	uint written = 0;
	
	if (resource == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Save Resource to Library! Error: Resource* was nullptr.");
		return 0;
	}

	char* buffer = nullptr;

	switch (resource->GetType())
	{
	case RESOURCE_TYPE::MODEL:		{ written = Importer::Scenes::Save((R_Model*)resource, &buffer); }			break;
	case RESOURCE_TYPE::MESH:		{ written = Importer::Meshes::Save((R_Mesh*)resource, &buffer); }			break;
	case RESOURCE_TYPE::MATERIAL:	{ written = Importer::Materials::Save((R_Material*)resource, &buffer); }	break;
	case RESOURCE_TYPE::TEXTURE:	{ written = Importer::Textures::Save((R_Texture*)resource, &buffer); }		break;
	case RESOURCE_TYPE::FOLDER:		{ written = Importer::Folders::Save((R_Folder*)resource, &buffer); }		break;
	case RESOURCE_TYPE::SCENE:		{ /*written = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }		break;
	case RESOURCE_TYPE::ANIMATION:	{ written = Importer::Animations::Save((R_Animation*)resource, &buffer); }	break;
	}

	RELEASE_ARRAY(buffer);

	if (written == 0)
	{
		LOG("[ERROR] Resource Manager: Could not save Resource in the Library! Error: Check for Importer Errors in the Console Panel.");
		return 0;
	}

	if (ResourceHasMetaType(resource))
	{
		SaveMetaFile(resource);
	}

	library.emplace(resource->GetUID(), resource->GetLibraryPath());

	return written;
}

const char* M_ResourceManager::GetValidPath(const char* assets_path)
{
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not validate the Assets Path! Error: Given path was nullptr.");
		return nullptr;
	}
	
	std::string norm_path = App->file_system->NormalizePath(assets_path);

	uint assets_path_start = norm_path.find("Assets");
	uint engine_path_start = norm_path.find("Engine");
	if (assets_path_start != std::string::npos)
	{
		norm_path = norm_path.substr(assets_path_start, norm_path.size());
		assets_path = _strdup(norm_path.c_str());
	}
	else if (engine_path_start != std::string::npos)
	{
		norm_path = norm_path.substr(engine_path_start, norm_path.size());
		assets_path = _strdup(norm_path.c_str());
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not validate the Assets Path! Error: Given path did not contain the Assets Directory");
		assets_path = nullptr;
	}

	return assets_path;
}

RESOURCE_TYPE M_ResourceManager::GetTypeFromAssetsExtension(const char* assets_path)
{
	RESOURCE_TYPE type = RESOURCE_TYPE::NONE;

	std::string extension = App->file_system->GetFileExtension(assets_path);

	if (extension == "fbx" || extension == "FBX" 
		|| extension == "obj" || extension == "OBJ")
	{
		type = RESOURCE_TYPE::MODEL;
	}
	else if (extension == "png" || extension == "PNG" 
			|| extension == "tga" || extension == "TGA" 
			|| extension == "dds" || extension == "DDS")
	{
		type = RESOURCE_TYPE::TEXTURE;
	}
	else if (extension == "json" || extension == "JSON")
	{
		type = RESOURCE_TYPE::SCENE;
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not import from the given Assets Path! Error: File extension is not supported!");
	}

	return type;
}

RESOURCE_TYPE M_ResourceManager::GetTypeFromLibraryExtension(const char* library_path)
{
	if (library_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get the Resource Type from the Library File's Extension! Error: Given Library Path was nullptr.");
		return RESOURCE_TYPE::NONE;
	}
	
	RESOURCE_TYPE type		= RESOURCE_TYPE::NONE;
	std::string extension	= App->file_system->GetFileExtension(library_path);
	extension				= "." + extension;

	if (extension == MODELS_EXTENSION)
	{
		type = RESOURCE_TYPE::MODEL;
	}
	else if (extension == MESHES_EXTENSION)
	{
		type = RESOURCE_TYPE::MESH;
	}
	else if (extension == MATERIALS_EXTENSION)
	{
		type = RESOURCE_TYPE::MATERIAL;
	}
	else if (extension == TEXTURES_EXTENSION)
	{
		type = RESOURCE_TYPE::TEXTURE;
	}
	else if (extension == FOLDERS_EXTENSION)
	{
		type = RESOURCE_TYPE::FOLDER;
	}
	else if (extension == SCENES_EXTENSION)
	{
		type = RESOURCE_TYPE::SCENE;
	}
	else if (extension == ANIMATIONS_EXTENSION)
	{
		type = RESOURCE_TYPE::ANIMATION;
	}
	else
	{
		type = RESOURCE_TYPE::NONE;
	}

	return type;
}

void M_ResourceManager::SetResourceAssetsPathAndFile(const char* assets_path, Resource* resource)
{
	resource->SetAssetsPath(assets_path);
	resource->SetAssetsFile(App->file_system->GetFileAndExtension(assets_path).c_str());
}

void M_ResourceManager::SetResourceLibraryPathAndFile(Resource* resource)
{
	resource->SetLibraryPathAndFile();
}

// --- META FILE METHODS ---
bool M_ResourceManager::SaveMetaFile(Resource* resource) const
{
	bool ret = true;

	if (resource == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Save the Meta File! Error: Given Resource* was nullptr.");
		return false;
	}

	ParsonNode meta_root = ParsonNode();
	meta_root.SetNumber("UID", resource->GetUID());																											// --- GENERAL RESOURCE META DATA
	meta_root.SetNumber("Type", (uint)resource->GetType());																									// 
	meta_root.SetString("Name", resource->GetAssetsFile());																									// 
	// ASSETS PATH?
	meta_root.SetString("LibraryPath", resource->GetLibraryPath());																							// 
	meta_root.SetNumber("ModificationTime", (double)App->file_system->GetLastModTime(resource->GetAssetsPath()));											// ------------------------------

	resource->SaveMeta(meta_root);																															// --- RESOURCE-SPECIFIC META DATA

	char* buffer		= nullptr;
	std::string path	= resource->GetAssetsPath() + std::string(META_EXTENSION);
	uint written		= meta_root.SerializeToFile(path.c_str(), &buffer);																					// --- SERIALIZING TO META FILE
	if (written > 0)
	{
		LOG("[STATUS] Resource Manager: Successfully Saved the Meta File for Resource %lu! Path: %s", resource->GetUID(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not Save the Meta File of Resource %lu! Error: File System could not write the file.", resource->GetUID());
	}

	RELEASE_ARRAY(buffer);

	return ret;
}

ParsonNode M_ResourceManager::LoadMetaFile(const char* assets_path, char** buffer)
{
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not load the .meta File! Error: Given path was nullptr!");
	}

	std::string meta_path	= assets_path + std::string(META_EXTENSION);
	uint read				= App->file_system->Load(meta_path.c_str(), buffer);
	if (read == 0)
	{
		LOG("[ERROR] Resource Manager: Could not load the .meta File with Path: %s! Error: No Meta File exists with the given path.", meta_path);
	}

	return ParsonNode(*buffer);
}

bool M_ResourceManager::HasMetaFile(const char* assets_path)
{
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not check whether or not the given path had an associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string meta_path = assets_path + std::string(META_EXTENSION);

	return App->file_system->Exists(meta_path.c_str());
}

bool M_ResourceManager::MetaFileIsValid(const char* assets_path)
{
	bool ret = true;

	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not validate Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string meta_path		= assets_path + std::string(META_EXTENSION);
	std::string error_string	= "[ERROR] Resource Manager: Could not validate Meta File " + meta_path;

	if (!App->file_system->Exists(meta_path.c_str()))
	{
		LOG("%s! Error: File System could not find the Meta File.", error_string.c_str());
		return false;
	}

	char* buffer					= nullptr;
	ParsonNode meta_root			= LoadMetaFile(assets_path, &buffer);
	ParsonArray contained_array		= meta_root.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node!", error_string.c_str());
		return false;
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ContainedResources Array from Meta Root!", error_string.c_str());
		return false;
	}

	std::string library_path	= meta_root.GetString("LibraryPath");
	uint32 resource_uid			= (uint32)meta_root.GetNumber("UID");
	if (!App->file_system->Exists(library_path.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", error_string.c_str());
		return false;
	}
	if (library.find(resource_uid) == library.end())
	{
		LOG("%s! Error: Resource UID could not be found in Library.", error_string.c_str());
		return false;
	}

	ParsonNode contained_node			= ParsonNode();
	uint32 contained_uid				= 0;
	std::string contained_library_path	= "[NONE]";
	for (uint i = 0; i < contained_array.size; ++i)
	{
		contained_node			= contained_array.GetNode(i);
		contained_uid			= (uint32)contained_node.GetNumber("UID");
		contained_library_path	= contained_node.GetString("LibraryPath");
		if (!App->file_system->Exists(contained_library_path.c_str()))
		{
			LOG("%s! Error: Contained Resource Custom File could not be found in Library.", error_string.c_str());
			return false;
		}
		if (library.find(contained_uid) == library.end())
		{
			LOG("%s! Error: Contained Resource UID could not be found in Library.", error_string.c_str());
			return false;
		}
	}

	return ret;
}

bool M_ResourceManager::MetaFileIsValid(ParsonNode& meta_root)
{
	bool ret = true;

	std::string error_string = "[ERROR] Resource Manager: Could not validate Meta File";

	ParsonArray contained_array = meta_root.GetArray("ContainedResources");
	if (!meta_root.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node!", error_string.c_str());
		return false;
	}
	if (!contained_array.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ContainedResources Array from Meta Root!", error_string.c_str());
		return false;
	}

	std::string library_path		= meta_root.GetString("LibraryPath");
	uint32 resource_uid				= (uint32)meta_root.GetNumber("UID");
	if (!App->file_system->Exists(library_path.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", error_string.c_str());
		return false;
	}
	if (library.find(resource_uid) == library.end())
	{
		LOG("%s! Error: Resource UID could not be found in Library.", error_string.c_str());
		return false;
	}

	ParsonNode contained_node			= ParsonNode();
	std::string contained_library_path	= "[NONE]";
	uint32 contained_uid				= 0;
	for (uint i = 0; i < contained_array.size; ++i)
	{
		contained_node = contained_array.GetNode(i);
		if (!contained_node.NodeIsValid())
		{
			LOG("%s! Error: Could not parse node %u from the Contained Array.", error_string.c_str(), i);
			return false;
		}

		contained_library_path	= contained_node.GetString("LibraryPath");
		contained_uid			= (uint32)contained_node.GetNumber("UID");
		if (!App->file_system->Exists(contained_library_path.c_str()))
		{
			LOG("%s! Error: Contained Resource Custom File could not be found in Library.", error_string.c_str());
			return false;
		}
		if (library.find(contained_uid) == library.end())
		{
			LOG("%s! Error: Contained Resource UID could not be found in Library.", error_string.c_str());
			return false;
		}
	}

	return ret;
}

bool M_ResourceManager::ResourceHasMetaType(Resource* resource) const
{
	if (resource == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not check that Resource* had Meta Type! Error: Resource* was nullptr.");
		return false;
	}
	
	RESOURCE_TYPE type = resource->GetType();
	
	return (type == RESOURCE_TYPE::FOLDER
			|| type == RESOURCE_TYPE::MODEL
			|| type == RESOURCE_TYPE::TEXTURE);
}

Resource* M_ResourceManager::GetResourceFromMetaFile(const char* assets_path)
{
	Resource* resource = nullptr;
	
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource associated with Meta File! Error: Given Assets Path was nullptr.");
		return nullptr;
	}

	std::string meta_file		= assets_path + std::string(META_EXTENSION);
	std::string error_string	= "[ERROR] Resource Manager: Could not get Resource associated with { " + meta_file + " } Meta File";

	if (!App->file_system->Exists(meta_file.c_str()))
	{
		LOG("%s! Error: File System could not find Meta File.", error_string.c_str());
		return nullptr;
	}

	uint32 resource_uid = LoadFromLibrary(assets_path);
	if (resource_uid == 0)
	{
		//LOG("%s! Error: Could not get Resource UID from Assets Path.", error_string.c_str());
		return nullptr;
	}

	resource = RequestResource(resource_uid);

	return resource;
}

// --- RESOURCE METHODS ---
Resource* M_ResourceManager::CreateResource(RESOURCE_TYPE type, const char* assets_path, const uint32& forced_UID)
{
	Resource* resource = nullptr;

	switch (type)
	{
	case RESOURCE_TYPE::MESH:		{ resource = new R_Mesh(); }		break;
	case RESOURCE_TYPE::MATERIAL:	{ resource = new R_Material(); }	break;
	case RESOURCE_TYPE::TEXTURE:	{ resource = new R_Texture(); }		break;
	case RESOURCE_TYPE::MODEL:		{ resource = new R_Model(); }		break;
	case RESOURCE_TYPE::FOLDER:		{ resource = new R_Folder(); }		break;
	case RESOURCE_TYPE::SCENE:		{ resource = new R_Scene(); }		break;
	case RESOURCE_TYPE::ANIMATION:	{ resource = new R_Animation(); }	break;
	}

	if (resource != nullptr)
	{
		if (assets_path != nullptr)
		{
			SetResourceAssetsPathAndFile(assets_path, resource);
		}

		if (forced_UID != 0)
		{
			resource->ForceUID(forced_UID);
		}

		SetResourceLibraryPathAndFile(resource);
	}

	return resource;
}

bool M_ResourceManager::DeleteResource(const uint32& UID)
{
	bool ret = true;
	
	if (library.find(UID) != library.end())
	{
		library.erase(UID);
	}

	Resource* resource_to_delete = nullptr;

	std::map<uint32, Resource*>::iterator item = resources.find(UID);
	if(item == resources.end())
	{
		LOG("[ERROR] Resource Manager: Resource to delete was not inside the resources std::map!");
		return false;
	}

	resource_to_delete = item->second;
	if (resource_to_delete != nullptr)
	{
		resource_to_delete->CleanUp();
		RELEASE(resource_to_delete);
	}

	resources.erase(UID);

	return ret;
}

bool M_ResourceManager::DeleteResource(Resource* resource_to_delete)
{
	if (resource_to_delete == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not delete given Resource! Error: Given Resource* was nullptr.");
		return false;
	}
	
	uint32 resource_uid = resource_to_delete->GetUID();

	resource_to_delete->CleanUp();
	RELEASE(resource_to_delete);

	if (library.find(resource_uid) != library.end())
	{
		library.erase(resource_uid);
	}
	if (resources.find(resource_uid) != resources.end())
	{
		resources.erase(resource_uid);
	}

	return true;
}

Resource* M_ResourceManager::RequestResource(const uint32& UID)
{	
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Resource Request Failed! Error: Requested UID was 0.");
		return nullptr;
	}
	
	Resource* resource		= nullptr;
	RESOURCE_ITEM r_item	= resources.find(UID);
	
	if (r_item != resources.end())
	{
		r_item->second->SetReferences(r_item->second->GetReferences() + 1);
		resource = r_item->second;
	}

	return resource;
}

bool M_ResourceManager::FreeResource(const uint32& UID)
{
	bool ret = true;
	
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Freed UID was 0.");
		return false;
	}
	
	RESOURCE_ITEM r_item = resources.find(UID);
	if (r_item != resources.end())
	{
		if (r_item->second->GetReferences() == 0)																					// Safety check in case resource already has 0 references.
		{
			DeallocateResource(UID);
		}
		
		r_item->second->SetReferences(r_item->second->GetReferences() - 1);

		if (r_item->second->GetReferences() == 0)																					// Deallocating the resource in case it has 0 references.
		{
			DeallocateResource(UID);
		}
	}
	else
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Given UID could not be found in Resources Map.");
		return false;
	}

	return ret;
}

Resource* M_ResourceManager::AllocateResource(const uint32& UID, const char* assets_path)
{
	if (assets_path == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not allocate Resource in Memory! Error: Given Path was nullptr.");
		return nullptr;
	}

	std::string error_string = "[ERROR] Resource Manager: Could not allocate Resource {" + std::string(assets_path) + "} in Memory";

	if (UID == 0)
	{
		LOG("%s! Error: Invalid UID.", error_string.c_str());
		return nullptr;
	}
	if (library.find(UID) == library.end())
	{
		LOG("%s! Error: Resource could not be found in Library.", error_string.c_str());
		return nullptr;
	}

	auto item = resources.find(UID);
	if (item != resources.end())
	{
		return item->second;
	}

	char* buffer				= nullptr;
	const char* library_path	= library.find(UID)->second.c_str();
	uint read					= App->file_system->Load(library_path, &buffer);
	if (read == 0)
	{
		LOG("%s! Error: File system could not read File [%s]", error_string.c_str(), library_path);
		return nullptr;
	}

	RESOURCE_TYPE type	= GetTypeFromLibraryExtension(library_path);
	Resource* resource	= CreateResource(type, assets_path, UID);
	bool success		= false;
	switch (type)
	{
	case RESOURCE_TYPE::MODEL:		{ success = Importer::Scenes::Load(buffer, (R_Model*)resource); }				break;
	case RESOURCE_TYPE::MESH:		{ success = Importer::Meshes::Load(buffer, (R_Mesh*)resource); }				break;
	case RESOURCE_TYPE::MATERIAL:	{ success = Importer::Materials::Load(buffer, (R_Material*)resource); }			break;
	case RESOURCE_TYPE::TEXTURE:	{ success = Importer::Textures::Load(buffer, read, (R_Texture*)resource); }		break;
	case RESOURCE_TYPE::FOLDER:		{ success = Importer::Folders::Load(buffer, (R_Folder*)resource); }				break;
	case RESOURCE_TYPE::SCENE:		{ /*success = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }			break;
	case RESOURCE_TYPE::ANIMATION:	{ success = Importer::Animations::Load(buffer, (R_Animation*)resource); }		break;
	}

	RELEASE_ARRAY(buffer);

	if (success)
	{
		resources.emplace(resource->GetUID(), resource);
		LOG("[STATUS] Resource Manager: Successfully Allocated Resource %s in Memory!", resource->GetAssetsFile());
	}
	else
	{
		DeleteResource(resource);																									// ATTENTION: This deletes from resources and library!.
		LOG("%s! Error: Importer could not load the Resource Data from File [%s].", error_string.c_str(), library_path);
	}

	return resource;
}

bool M_ResourceManager::DeallocateResource(const uint32& UID)
{
	bool ret = true;
	
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource! Error: Given UID was 0");
		return false;
	}

	RESOURCE_ITEM item = resources.find(UID);
	if (item != resources.end())
	{
		item->second->CleanUp();
		RELEASE(item->second);
		resources.erase(UID);
		return true;
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource %lu! Error: Resource was not allocated in memory.", UID);
	}

	return ret;
}

bool M_ResourceManager::DeallocateResource(Resource* resource_to_deallocate)
{
	bool ret = true;

	if (resource_to_deallocate == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not deallocate Resource! Error: Given Resource* was nullptr");
		return false;
	}

	uint32 resource_uid = resource_to_deallocate->GetUID();

	resource_to_deallocate->CleanUp();
	RELEASE(resource_to_deallocate);

	if (resources.find(resource_uid) != resources.end())
	{
		resources.erase(resource_uid);
	}
	else
	{
		//LOG("[WARNING] Resource Manager: Deallocated Resource was not stored in the Resources Map!");
	}

	return ret;
}

void M_ResourceManager::GetResources(std::map<uint32, Resource*>& resources) const
{
	resources = this->resources;
}