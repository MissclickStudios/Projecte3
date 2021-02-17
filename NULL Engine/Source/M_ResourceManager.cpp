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
fileRefreshTimer	(0.0f),
fileRefreshRate	(0.0f)
{

}

M_ResourceManager::~M_ResourceManager()
{

}
 
bool M_ResourceManager::Init(ParsonNode& configuration)
{
	bool ret = true;

	//file_refresh_rate = (float)configuration.GetNumber("RefreshRate");
	fileRefreshRate = 5.0f;

	return ret;
}

bool M_ResourceManager::Start()
{
	bool ret = true;

	RefreshDirectoryFiles(ASSETS_DIRECTORY);
	RefreshDirectoryFiles(ENGINE_DIRECTORY);

	return ret;
}

UpdateStatus M_ResourceManager::PreUpdate(float dt)
{
	UpdateStatus status = UpdateStatus::CONTINUE;

	fileRefreshTimer += Time::Real::GetDT();

	if (fileRefreshTimer > fileRefreshRate)
	{
		RESOURCE_ITEM item = resources.begin();
		while (item != resources.end())
		{
			if (item->second->GetReferences() == 0)															// Clear all Reference 0 resources that might have gone past the 
			{																								// FreeResource() method.
				uint32 resourceUid = item->second->GetUID();												// 
				++item;																						// Setting item to the next element so the reference is not lost after
				DeallocateResource(resourceUid);															// erasing the element with the resource_uid from the resources std::map.
				continue;																					// Going to the next iteration so item is not updated twice in the same loop.
			}

			++item;
		}
		
		//RefreshDirectoryFiles(ASSETS_DIRECTORY);

		fileRefreshTimer = 0.0f;
	}

	return status;
}

UpdateStatus M_ResourceManager::Update(float dt)
{
	UpdateStatus status = UpdateStatus::CONTINUE;

	

	return status;
}

UpdateStatus M_ResourceManager::PostUpdate(float dt)
{
	UpdateStatus status = UpdateStatus::CONTINUE;



	return status;
}

bool M_ResourceManager::CleanUp()
{
	bool ret = true;

	std::map<uint32, Resource*>::iterator rItem;
	for (rItem = resources.begin(); rItem != resources.end(); ++rItem)
	{
		rItem->second->CleanUp();
		RELEASE(rItem->second);
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
	
	std::vector<std::string> filesToImport;															// Refresh folders and create .meta files for all those files that do not have one.
	std::vector<std::string> filesToUpdate;															// Also update the .meta and reimport all those files that have been modified.
	std::vector<std::string> filesToDelete;

	RefreshDirectory(directory, filesToImport, filesToUpdate, filesToDelete);

	for (uint i = 0; i < filesToDelete.size(); ++i)
	{
		//DeleteFromAssets(files_to_delete[i].c_str());
		DeleteFromLibrary(filesToDelete[i].c_str());
	}
	for (uint i = 0; i < filesToUpdate.size(); ++i)
	{
		DeleteFromLibrary(filesToUpdate[i].c_str());
		ImportFile(filesToUpdate[i].c_str());
	}
	for (uint i = 0; i < filesToImport.size(); ++i)
	{
		ImportFile(filesToImport[i].c_str());
	}

	filesToDelete.clear();
	filesToUpdate.clear();
	filesToImport.clear();
}

void M_ResourceManager::RefreshDirectory(const char* directory, std::vector<std::string>& filesToImport, 
												std::vector<std::string>& filesToUpdate, std::vector<std::string>& filesToDelete)
{
	if (directory == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Refresh Directory! Error: Directory string was nullptr.");
		return;
	}
	
	std::vector<std::string> directories;
	std::vector<std::string> assetFiles;
	std::vector<std::string> metaFiles;
	std::map<std::string, std::string> filePairs;

	App->fileSystem->DiscoverAllFiles(directory, assetFiles, directories, DOTLESS_META_EXTENSION);				// Directories (folders) will be ignored for now.
	App->fileSystem->GetAllFilesWithExtension(directory, DOTLESS_META_EXTENSION, metaFiles);
	
	FindFilesToImport(assetFiles, metaFiles, filePairs, filesToImport);										// Always call in this order!
	FindFilesToUpdate(filePairs, filesToUpdate);																	// At the very least FindFilesToImport() has to be the first to be called
	FindFilesToDelete(metaFiles, filePairs, filesToDelete);														// as it is the one to fill file_pairs with asset and meta files!

	LoadValidFilesIntoLibrary(filePairs);																			// Will emplace all valid files' UID & library path into the library map.

	filePairs.clear();
	metaFiles.clear();
	assetFiles.clear();
	directories.clear();
}

void M_ResourceManager::FindFilesToImport(const std::vector<std::string>& assetFiles, const std::vector<std::string>& metaFiles, 
											std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToImport)
{
	std::map<std::string, uint> metaTmp;
	for (uint i = 0; i < metaFiles.size(); ++i)																	// Adding the meta_files to a map so the elements can be found with ease.
	{
		metaTmp.emplace(metaFiles[i], i);
	}
	
	std::string metaFile = "[NONE]";
	std::map<std::string, uint>::iterator item;
	for (uint i = 0; i < assetFiles.size(); ++i)																	// Assets files whose meta file is missing will be imported.
	{
		metaFile	= assetFiles[i] + META_EXTENSION;
		item		= metaTmp.find(metaFile);

		if (item != metaTmp.end() /*&& MetaFileIsValid(asset_files[i].c_str())*/)
		{
			filePairs.emplace(assetFiles[i], item->first);

			if (!MetaFileIsValid(assetFiles[i].c_str()))															// In case the pair exists but the meta file is outdated.
			{
				filesToImport.push_back(assetFiles[i]);
			}
		}
		else
		{
			filesToImport.push_back(assetFiles[i]);
		}
	}

	metaTmp.clear();
}

void M_ResourceManager::FindFilesToUpdate(const std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToUpdate)
{
	uint64 assetModTime	= 0;
	uint64 metaModTime	= 0;
	std::map<std::string, std::string>::const_iterator item;
	for (item = filePairs.begin(); item != filePairs.end(); ++item)
	{
		assetModTime	= App->fileSystem->GetLastModTime(item->first.c_str());						// Files with different modification time than their meta files
		metaModTime	= GetAssetFileModTimeFromMeta(item->first.c_str());								// will need to be updated (Delete prev + Import new).

		if (assetModTime != metaModTime)
		{
			filesToUpdate.push_back(item->first);
		}
	}
}

void M_ResourceManager::FindFilesToDelete(const std::vector<std::string>& metaFiles, const std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToDelete)
{
	std::string assetsPath = "[NONE]";
	for (uint i = 0; i < metaFiles.size(); ++i)																	// Meta files whose asset_file is missing will be deleted.
	{
		assetsPath = metaFiles[i];
		assetsPath.resize(assetsPath.size() - std::string(META_EXTENSION).size());								// Dirty-but-quick way of eliminating the extension.

 		if (filePairs.find(assetsPath) == filePairs.end())														// If cannot find assets_path in the paired files.
		{						
			filesToDelete.push_back(assetsPath);
		}
	}
}

void M_ResourceManager::LoadValidFilesIntoLibrary(const std::map<std::string, std::string>& filePairs)
{
	std::map<std::string, std::string>::const_iterator item;
	for (item = filePairs.cbegin(); item != filePairs.cend(); ++item)
	{
		LoadMetaLibraryPairsIntoLibrary(item->first.c_str());
	}
}

bool M_ResourceManager::DeleteFromAssets(const char* assetsPath)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Delete File from Assets! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::vector<uint32> resourceUids;
	std::vector<std::string> filesToDelete;

	GetResourceUIDsFromMeta(assetsPath, resourceUids);
	GetLibraryFilePathsFromMeta(assetsPath, filesToDelete);
	
	std::string metaPath = assetsPath + std::string(META_EXTENSION);
	filesToDelete.push_back(assetsPath);
	filesToDelete.push_back(metaPath);

	for (uint i = 0; i < resourceUids.size(); ++i)
	{
		DeleteResource(resourceUids[i]);
	}

	for (uint i = 0; i < filesToDelete.size(); ++i)
	{
		/*ret =*/ App->fileSystem->Remove(filesToDelete[i].c_str());
	}

	filesToDelete.clear();
	resourceUids.clear();

	return ret;
}

bool M_ResourceManager::DeleteFromLibrary(const char* assetsPath)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not delete Files from Library! Error: Given Assets Path was nullptr.");
		return false;
	}

	if (!HasMetaFile(assetsPath))
	{
		LOG("[ERROR] Resource Manager: Could not delete { %s }'s Files from Library! Error: Given Assets Path had no associated Meta File.", assetsPath);
		return false;
	}

	std::vector<uint32> resourceUids;
	std::vector<std::string> filesToDelete;

	GetResourceUIDsFromMeta(assetsPath, resourceUids);
	GetLibraryFilePathsFromMeta(assetsPath, filesToDelete);

	for (uint i = 0; i < resourceUids.size(); ++i)
	{
		DeleteResource(resourceUids[i]);
	}

	for (uint i = 0; i < filesToDelete.size(); ++i)
	{
		App->fileSystem->Remove(filesToDelete[i].c_str());
	}

	filesToDelete.clear();
	resourceUids.clear();

	return ret;
}

bool M_ResourceManager::GetResourceUIDsFromMeta(const char* assetsPath, std::vector<uint32>& resourceUids)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource UIDs from Meta! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not get Resource UIDs from { " + std::string(assetsPath) + " }'s Meta";

	char* buffer					= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray		= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no correspondent Meta File.", errorString.c_str());
		return false;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: Contained Array in Meta File was not valid.", errorString.c_str());
		return false;
	}

	// --- MAIN RESOURCE
	uint32 resourceUid = (uint32)metaRoot.GetNumber("UID");
	if (resourceUid == 0)
	{
		LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		return false;
	}

	resourceUids.push_back(resourceUid);

	// --- CONTAINED RESOURCES
	uint32 containedUid		= 0;
	ParsonNode containedNode	= ParsonNode();
	for (uint i = 0; i < containedArray.size; ++i)
	{
		containedNode = containedArray.GetNode(i);
		if (!containedNode.NodeIsValid())
		{
			continue;
		}

		containedUid = 0;																									// Resetting the contained uid
		containedUid = (uint32)containedNode.GetNumber("UID");
		if (containedUid == 0)
		{
			LOG("[WARNING] Resource Manager: Contained UID was not valid!");
			continue;
		}

		resourceUids.push_back(containedUid);
	}

	return ret;
}

bool M_ResourceManager::GetLibraryFilePathsFromMeta(const char* assetsPath, std::vector<std::string>& filePaths)
{
	bool ret = true;
	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}
	
	std::string errorString = "[ERROR] Resoruce Manager: Could not get Library File Paths from { " + std::string(assetsPath) + " }'s Meta File";

	char* buffer					= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray		= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: ContainedResources array in Meta File was not valid.", errorString.c_str());
		return false;
	}

	std::string directory = "[NONE]";
	std::string extension = "[NONE]";

	// --- MAIN RESOURCE
	uint32 resourceUid		= (uint32)metaRoot.GetNumber("UID");
	ResourceType type		= (ResourceType)metaRoot.GetNumber("Type");
	bool success			= GetLibraryDirectoryAndExtensionFromType(type, directory, extension);
	if (!success)
	{
		LOG("%s! Error: Could not get the Library Directory and Extension from Resource Type.", errorString.c_str());
		return false;
	}
	if (resourceUid == 0)
	{
		LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		return false;
	}
	if (directory == "[NONE]" || extension == "[NONE]")
	{
		LOG("%s! Error: Main Resource Library Directory or Extension strings were not valid.", errorString.c_str());
		return false;
	}

	std::string libraryPath = directory + std::to_string(resourceUid) + extension;
	filePaths.push_back(libraryPath);

	// --- CONTAINED RESOURCES
	ParsonNode containedNode		= ParsonNode();
	uint32 containedUid			= 0;
	ResourceType containedType	= ResourceType::NONE;
	std::string containedPath		= "[NONE]";
	for (uint i = 0; i < containedArray.size; ++i)
	{
		containedNode = containedArray.GetNode(i);
		if (!containedNode.NodeIsValid())
		{
			continue;
		}

		directory = "[NONE]";
		extension = "[NONE]";

		containedUid	= (uint32)containedNode.GetNumber("UID");
		containedType	= (ResourceType)containedNode.GetNumber("Type");
		success			= GetLibraryDirectoryAndExtensionFromType(containedType, directory, extension);
		if (!success)
		{
			continue;
		}
		if (containedUid == 0)
		{
			continue;
		}
		if (directory == "[NONE]" || extension == "[NONE]")
		{
			continue;
		}

		containedPath = directory + std::to_string(containedUid) + extension;
		filePaths.push_back(containedPath);
	}

	return ret;
}

bool M_ResourceManager::GetLibraryDirectoryAndExtensionFromType(const ResourceType& type, std::string& directory, std::string& extension)
{
	bool ret = true;
	
	switch (type)
	{
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
	case ResourceType::FOLDER:
		directory = FOLDERS_PATH;
		extension = FOLDERS_EXTENSION;
		break;
	case ResourceType::SCENE:
		directory = SCENES_PATH;
		extension = SCENES_EXTENSION;
		break;
	case ResourceType::ANIMATION:
		directory = ANIMATIONS_PATH;
		extension = ANIMATIONS_EXTENSION;
		break;
	case ResourceType::NONE:
		ret = false;
		break;
	}

	return ret;
}

bool M_ResourceManager::LoadMetaLibraryPairsIntoLibrary(const char* assetsPath)
{
	bool ret = true;
	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not load Meta Library Pairs into Library! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::map<uint32, std::string> libraryPairs;
	GetLibraryPairsFromMeta(assetsPath, libraryPairs);

	std::map<uint32, std::string>::iterator item;
	for (item = libraryPairs.begin(); item != libraryPairs.end(); ++item)
	{
		library.emplace(item->first, item->second);
	}

	libraryPairs.clear();

	return ret;
}

bool M_ResourceManager::GetLibraryPairsFromMeta(const char* assetsPath, std::map<uint32, std::string>& pairs)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library Pairs from File's associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not get Libarary Pairs from { " + std::string(assetsPath) + " }'s associated Meta File";

	std::vector<uint32> resourceUids;
	std::vector<std::string> libraryPaths;

	GetResourceUIDsFromMeta(assetsPath, resourceUids);
	GetLibraryFilePathsFromMeta(assetsPath, libraryPaths);

	if (resourceUids.size() != libraryPaths.size())
	{
		LOG("%s! Error: Mismatched amount of Resource UIDs and Library Paths.", errorString.c_str());
	}

	for (uint i = 0; i < resourceUids.size(); ++i)
	{
		pairs.emplace(resourceUids[i], libraryPaths[i]);
	}

	resourceUids.clear();
	libraryPaths.clear();

	return ret;
}

uint64 M_ResourceManager::GetAssetFileModTimeFromMeta(const char* assetsPath)
{
	uint64 ret = 0;
	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string errorString = "[ERROR] Resoruce Manager: Could not get Asset File Mod Time from { " + std::string(assetsPath) + " }'s Meta File";

	char* buffer					= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray		= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
		return 0;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: ContainedResources array in Meta File was not valid.", errorString.c_str());
		return 0;
	}

	ret = (uint64)metaRoot.GetNumber("ModificationTime");

	return ret;
}

// --- IMPORT FILE METHODS--
uint32 M_ResourceManager::ImportFile(const char* assetsPath)
{
	uint32 resourceUid = 0;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: Path was nullptr.");
		return 0;
	}

	if (App->fileSystem->GetFileExtension(assetsPath) == "json" 
		|| App->fileSystem->GetFileExtension(assetsPath) == "JSON")													// TMP until R_Scene has been fully implemented.
	{
		return 0;
	}

	assetsPath			= GetValidPath(assetsPath);
	bool metaIsValid	= MetaFileIsValid(assetsPath);
	if (!metaIsValid)
	{
		if (HasMetaFile(assetsPath))
		{
			DeleteFromLibrary(assetsPath);																				// Cleaning any remaining Library files.
		}
		
		resourceUid = ImportFromAssets(assetsPath); 

		if (resourceUid == 0)
		{
			LOG("[ERROR] Resource Manager: Could not Import File { %s }! Error: See [IMPORTER] and Resource Manager ERRORS.", assetsPath);
			return 0;
		}
	}
	else
	{
		LOG("[WARNING] Resource Manager: The File to Import was already in the Library!");

		std::map<uint32, std::string> libraryItems;
		GetLibraryPairsFromMeta(assetsPath, libraryItems);

		std::map<uint32, std::string>::iterator item;
		for (item = libraryItems.begin(); item != libraryItems.end(); ++item)
		{
			if (library.find(item->first) == library.end())
			{
				library.emplace(item->first, item->second);
			}
		}

		resourceUid = libraryItems.begin()->first;

		libraryItems.clear();
	}

	return resourceUid;
}

uint32 M_ResourceManager::ImportFromAssets(const char* assetsPath)
{
	uint32 resourceUid = 0;
	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: Path was nullptr.");
		return 0;
	}

	char* buffer = nullptr;
	uint read = App->fileSystem->Load(assetsPath, &buffer);
	if (read > 0)
	{
		ResourceType type = GetTypeFromAssetsExtension(assetsPath);
		Resource* resource = CreateResource(type, assetsPath);

		bool success = false;
		switch (type)
		{
		case ResourceType::MODEL:		{ success = Importer::ImportScene(buffer, read, (R_Model*)resource); }		break;
		case ResourceType::MESH:		{ success = Importer::ImportMesh(buffer, (R_Mesh*)resource); }				break;
		case ResourceType::TEXTURE:	{ success = Importer::ImportTexture(buffer, read, (R_Texture*)resource); }	break;
		case ResourceType::SCENE:		{ /*success = HAVE A FUNCTIONAL R_SCENE AND LOAD/SAVE METHODS*/}			break;
		}

		RELEASE_ARRAY(buffer);

		if (!success)
		{
			LOG("[ERROR] Resource Manager: Could not Import File %s! Error: Check for [IMPORTER] errors in the Console Panel.", assetsPath);
			DeallocateResource(resource);
			return 0;
		}

		resourceUid = resource->GetUID();
		SaveResourceToLibrary(resource);
		DeallocateResource(resource);
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not Import File %s! Error: File System could not Read the File.", assetsPath);
		return 0;
	}

	return resourceUid;
}

uint32 M_ResourceManager::LoadFromLibrary(const char* assetsPath)
{
	uint32 resourceUid = 0;
	
	std::string errorString = "[ERROR] Resource Manager: Could not Load File from the given Library Path";

	assetsPath = GetValidPath(assetsPath);

	if (assetsPath == nullptr)
	{
		LOG("%s! Error: Path was nullptr.", errorString.c_str());
		return 0;
	}

	char* buffer			= nullptr;
	ParsonNode metaRoot	= LoadMetaFile(assetsPath, &buffer);
	RELEASE_ARRAY(buffer);

	bool metaIsValid		= MetaFileIsValid(metaRoot);
	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node.", errorString.c_str());
		return 0;
	}
	if (!metaIsValid)
	{
		LOG("%s! Error: Could not Validate the Meta File.", errorString.c_str());
		return 0;
	}

	resourceUid					= (uint32)metaRoot.GetNumber("UID");
	ParsonArray containedArray		= metaRoot.GetArray("ContainedResources");
	
	if (resources.find(resourceUid) != resources.end())
	{
		return resourceUid;																									// If the File To Load's Resource is already in memory.
	}	

	Resource* result	= nullptr;
	result				= AllocateResource(resourceUid, assetsPath);
	if (result == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Allocate Resource %lu in memory!", resourceUid);
		return 0;
	}

	std::string containedPath = "";
	std::string containedName = "";
	for (uint i = 0; i < containedArray.size; ++i)
	{
		ParsonNode containedNode = containedArray.GetNode(i);

		App->fileSystem->SplitFilePath(assetsPath, &containedPath, nullptr, nullptr);									// --- TMP Until Something Functional Is In Place.
		containedName	= containedNode.GetString("Name");																	// 
		containedPath += containedName;																					// -----------------------------------------------

		uint32 containedUid = (uint32)containedNode.GetNumber("UID");

		if (resources.find(containedUid) != resources.end())																// No need to allocate if it is already allocated.
		{
			continue;
		}

		result = AllocateResource(containedUid, containedPath.c_str());
		if (result == nullptr)
		{
			LOG("[WARNING] Resource Manager: Could not allocate Contained Resource! UID: %lu, Name: %s", containedUid, containedName.c_str());
		}

		containedName.clear();
		containedPath.clear();
	}

	return resourceUid;
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
	case ResourceType::MODEL:		{ written = Importer::Scenes::Save((R_Model*)resource, &buffer); }			break;
	case ResourceType::MESH:		{ written = Importer::Meshes::Save((R_Mesh*)resource, &buffer); }			break;
	case ResourceType::MATERIAL:	{ written = Importer::Materials::Save((R_Material*)resource, &buffer); }	break;
	case ResourceType::TEXTURE:	{ written = Importer::Textures::Save((R_Texture*)resource, &buffer); }		break;
	case ResourceType::FOLDER:		{ written = Importer::Folders::Save((R_Folder*)resource, &buffer); }		break;
	case ResourceType::SCENE:		{ /*written = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }		break;
	case ResourceType::ANIMATION:	{ written = Importer::Animations::Save((R_Animation*)resource, &buffer); }	break;
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

const char* M_ResourceManager::GetValidPath(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not validate the Assets Path! Error: Given path was nullptr.");
		return nullptr;
	}
	
	std::string normPath = App->fileSystem->NormalizePath(assetsPath);

	uint assetsPathStart = normPath.find("Assets");
	uint enginePathStart = normPath.find("Engine");
	if (assetsPathStart != std::string::npos)
	{
		normPath = normPath.substr(assetsPathStart, normPath.size());
		assetsPath = _strdup(normPath.c_str());
	}
	else if (enginePathStart != std::string::npos)
	{
		normPath = normPath.substr(enginePathStart, normPath.size());
		assetsPath = _strdup(normPath.c_str());
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not validate the Assets Path! Error: Given path did not contain the Assets Directory");
		assetsPath = nullptr;
	}

	return assetsPath;
}

ResourceType M_ResourceManager::GetTypeFromAssetsExtension(const char* assetsPath)
{
	ResourceType type = ResourceType::NONE;

	std::string extension = App->fileSystem->GetFileExtension(assetsPath);

	if (extension == "fbx" || extension == "FBX" 
		|| extension == "obj" || extension == "OBJ")
	{
		type = ResourceType::MODEL;
	}
	else if (extension == "png" || extension == "PNG" 
			|| extension == "tga" || extension == "TGA" 
			|| extension == "dds" || extension == "DDS")
	{
		type = ResourceType::TEXTURE;
	}
	else if (extension == "json" || extension == "JSON")
	{
		type = ResourceType::SCENE;
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not import from the given Assets Path! Error: File extension is not supported!");
	}

	return type;
}

ResourceType M_ResourceManager::GetTypeFromLibraryExtension(const char* libraryPath)
{
	if (libraryPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get the Resource Type from the Library File's Extension! Error: Given Library Path was nullptr.");
		return ResourceType::NONE;
	}
	
	ResourceType type		= ResourceType::NONE;
	std::string extension	= App->fileSystem->GetFileExtension(libraryPath);
	extension				= "." + extension;

	if (extension == MODELS_EXTENSION)
	{
		type = ResourceType::MODEL;
	}
	else if (extension == MESHES_EXTENSION)
	{
		type = ResourceType::MESH;
	}
	else if (extension == MATERIALS_EXTENSION)
	{
		type = ResourceType::MATERIAL;
	}
	else if (extension == TEXTURES_EXTENSION)
	{
		type = ResourceType::TEXTURE;
	}
	else if (extension == FOLDERS_EXTENSION)
	{
		type = ResourceType::FOLDER;
	}
	else if (extension == SCENES_EXTENSION)
	{
		type = ResourceType::SCENE;
	}
	else if (extension == ANIMATIONS_EXTENSION)
	{
		type = ResourceType::ANIMATION;
	}
	else
	{
		type = ResourceType::NONE;
	}

	return type;
}

void M_ResourceManager::SetResourceAssetsPathAndFile(const char* assetsPath, Resource* resource)
{
	resource->SetAssetsPath(assetsPath);
	resource->SetAssetsFile(App->fileSystem->GetFileAndExtension(assetsPath).c_str());
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

	ParsonNode metaRoot = ParsonNode();
	metaRoot.SetNumber("UID", resource->GetUID());																											// --- GENERAL RESOURCE META DATA
	metaRoot.SetNumber("Type", (uint)resource->GetType());																									// 
	metaRoot.SetString("Name", resource->GetAssetsFile());																									// 
	// ASSETS PATH?
	metaRoot.SetString("LibraryPath", resource->GetLibraryPath());																							// 
	metaRoot.SetNumber("ModificationTime", (double)App->fileSystem->GetLastModTime(resource->GetAssetsPath()));											// ------------------------------

	resource->SaveMeta(metaRoot);																															// --- RESOURCE-SPECIFIC META DATA

	char* buffer		= nullptr;
	std::string path	= resource->GetAssetsPath() + std::string(META_EXTENSION);
	uint written		= metaRoot.SerializeToFile(path.c_str(), &buffer);																					// --- SERIALIZING TO META FILE
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

ParsonNode M_ResourceManager::LoadMetaFile(const char* assetsPath, char** buffer)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not load the .meta File! Error: Given path was nullptr!");
	}

	std::string metaPath	= assetsPath + std::string(META_EXTENSION);
	uint read				= App->fileSystem->Load(metaPath.c_str(), buffer);
	if (read == 0)
	{
		LOG("[ERROR] Resource Manager: Could not load the .meta File with Path: %s! Error: No Meta File exists with the given path.", metaPath);
	}

	return ParsonNode(*buffer);
}

bool M_ResourceManager::HasMetaFile(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not check whether or not the given path had an associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string metaPath = assetsPath + std::string(META_EXTENSION);

	return App->fileSystem->Exists(metaPath.c_str());
}

bool M_ResourceManager::MetaFileIsValid(const char* assetsPath)
{
	bool ret = true;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not validate Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string metaPath		= assetsPath + std::string(META_EXTENSION);
	std::string errorString	= "[ERROR] Resource Manager: Could not validate Meta File " + metaPath;

	if (!App->fileSystem->Exists(metaPath.c_str()))
	{
		LOG("%s! Error: File System could not find the Meta File.", errorString.c_str());
		return false;
	}

	char* buffer					= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray		= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node!", errorString.c_str());
		return false;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ContainedResources Array from Meta Root!", errorString.c_str());
		return false;
	}

	std::string libraryPath	= metaRoot.GetString("LibraryPath");
	uint32 resourceUid			= (uint32)metaRoot.GetNumber("UID");
	if (!App->fileSystem->Exists(libraryPath.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", errorString.c_str());
		return false;
	}
	if (library.find(resourceUid) == library.end())
	{
		LOG("%s! Error: Resource UID could not be found in Library.", errorString.c_str());
		return false;
	}

	ParsonNode containedNode			= ParsonNode();
	uint32 containedUid				= 0;
	std::string containedLibraryPath	= "[NONE]";
	for (uint i = 0; i < containedArray.size; ++i)
	{
		containedNode			= containedArray.GetNode(i);
		containedUid			= (uint32)containedNode.GetNumber("UID");
		containedLibraryPath	= containedNode.GetString("LibraryPath");
		if (!App->fileSystem->Exists(containedLibraryPath.c_str()))
		{
			LOG("%s! Error: Contained Resource Custom File could not be found in Library.", errorString.c_str());
			return false;
		}
		if (library.find(containedUid) == library.end())
		{
			LOG("%s! Error: Contained Resource UID could not be found in Library.", errorString.c_str());
			return false;
		}
	}

	return ret;
}

bool M_ResourceManager::MetaFileIsValid(ParsonNode& metaRoot)
{
	bool ret = true;

	std::string errorString = "[ERROR] Resource Manager: Could not validate Meta File";

	ParsonArray containedArray = metaRoot.GetArray("ContainedResources");
	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Could not get the Meta Root Node!", errorString.c_str());
		return false;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: Could not get the ContainedResources Array from Meta Root!", errorString.c_str());
		return false;
	}

	std::string libraryPath		= metaRoot.GetString("LibraryPath");
	uint32 resourceUid				= (uint32)metaRoot.GetNumber("UID");
	if (!App->fileSystem->Exists(libraryPath.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", errorString.c_str());
		return false;
	}
	if (library.find(resourceUid) == library.end())
	{
		LOG("%s! Error: Resource UID could not be found in Library.", errorString.c_str());
		return false;
	}

	ParsonNode containedNode			= ParsonNode();
	std::string containedLibraryPath	= "[NONE]";
	uint32 containedUid				= 0;
	for (uint i = 0; i < containedArray.size; ++i)
	{
		containedNode = containedArray.GetNode(i);
		if (!containedNode.NodeIsValid())
		{
			LOG("%s! Error: Could not parse node %u from the Contained Array.", errorString.c_str(), i);
			return false;
		}

		containedLibraryPath	= containedNode.GetString("LibraryPath");
		containedUid			= (uint32)containedNode.GetNumber("UID");
		if (!App->fileSystem->Exists(containedLibraryPath.c_str()))
		{
			LOG("%s! Error: Contained Resource Custom File could not be found in Library.", errorString.c_str());
			return false;
		}
		if (library.find(containedUid) == library.end())
		{
			LOG("%s! Error: Contained Resource UID could not be found in Library.", errorString.c_str());
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
	
	ResourceType type = resource->GetType();
	
	return (type == ResourceType::FOLDER
			|| type == ResourceType::MODEL
			|| type == ResourceType::TEXTURE);
}

Resource* M_ResourceManager::GetResourceFromMetaFile(const char* assetsPath)
{
	Resource* resource = nullptr;
	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource associated with Meta File! Error: Given Assets Path was nullptr.");
		return nullptr;
	}

	std::string metaFile		= assetsPath + std::string(META_EXTENSION);
	std::string errorString	= "[ERROR] Resource Manager: Could not get Resource associated with { " + metaFile + " } Meta File";

	if (!App->fileSystem->Exists(metaFile.c_str()))
	{
		LOG("%s! Error: File System could not find Meta File.", errorString.c_str());
		return nullptr;
	}

	uint32 resourceUid = LoadFromLibrary(assetsPath);
	if (resourceUid == 0)
	{
		//LOG("%s! Error: Could not get Resource UID from Assets Path.", error_string.c_str());
		return nullptr;
	}

	resource = RequestResource(resourceUid);

	return resource;
}

// --- RESOURCE METHODS ---
Resource* M_ResourceManager::CreateResource(ResourceType type, const char* assetsPath, const uint32& forcedUid)
{
	Resource* resource = nullptr;

	switch (type)
	{
	case ResourceType::MESH:		{ resource = new R_Mesh(); }		break;
	case ResourceType::MATERIAL:	{ resource = new R_Material(); }	break;
	case ResourceType::TEXTURE:	{ resource = new R_Texture(); }		break;
	case ResourceType::MODEL:		{ resource = new R_Model(); }		break;
	case ResourceType::FOLDER:		{ resource = new R_Folder(); }		break;
	case ResourceType::SCENE:		{ resource = new R_Scene(); }		break;
	case ResourceType::ANIMATION:	{ resource = new R_Animation(); }	break;
	}

	if (resource != nullptr)
	{
		if (assetsPath != nullptr)
		{
			SetResourceAssetsPathAndFile(assetsPath, resource);
		}

		if (forcedUid != 0)
		{
			resource->ForceUID(forcedUid);
		}

		SetResourceLibraryPathAndFile(resource);
	}

	return resource;
}

bool M_ResourceManager::DeleteResource(const uint32& uid)
{
	bool ret = true;
	
	if (library.find(uid) != library.end())
	{
		library.erase(uid);
	}

	Resource* resourceToDelete = nullptr;

	std::map<uint32, Resource*>::iterator item = resources.find(uid);
	if(item == resources.end())
	{
		LOG("[ERROR] Resource Manager: Resource to delete was not inside the resources std::map!");
		return false;
	}

	resourceToDelete = item->second;
	if (resourceToDelete != nullptr)
	{
		resourceToDelete->CleanUp();
		RELEASE(resourceToDelete);
	}

	resources.erase(uid);

	return ret;
}

bool M_ResourceManager::DeleteResource(Resource* resourceToDelete)
{
	if (resourceToDelete == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not delete given Resource! Error: Given Resource* was nullptr.");
		return false;
	}
	
	uint32 resourceUid = resourceToDelete->GetUID();

	resourceToDelete->CleanUp();
	RELEASE(resourceToDelete);

	if (library.find(resourceUid) != library.end())
	{
		library.erase(resourceUid);
	}
	if (resources.find(resourceUid) != resources.end())
	{
		resources.erase(resourceUid);
	}

	return true;
}

Resource* M_ResourceManager::RequestResource(const uint32& uid)
{	
	if (uid == 0)
	{
		LOG("[ERROR] Resource Manager: Resource Request Failed! Error: Requested UID was 0.");
		return nullptr;
	}
	
	Resource* resource		= nullptr;
	RESOURCE_ITEM rItem	= resources.find(uid);
	
	if (rItem != resources.end())
	{
		rItem->second->SetReferences(rItem->second->GetReferences() + 1);
		resource = rItem->second;
	}

	return resource;
}

bool M_ResourceManager::FreeResource(const uint32& uid)
{
	bool ret = true;
	
	if (uid == 0)
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Freed UID was 0.");
		return false;
	}
	
	RESOURCE_ITEM rItem = resources.find(uid);
	if (rItem != resources.end())
	{
		if (rItem->second->GetReferences() == 0)																					// Safety check in case resource already has 0 references.
		{
			DeallocateResource(uid);
		}
		
		rItem->second->SetReferences(rItem->second->GetReferences() - 1);

		if (rItem->second->GetReferences() == 0)																					// Deallocating the resource in case it has 0 references.
		{
			DeallocateResource(uid);
		}
	}
	else
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Given UID could not be found in Resources Map.");
		return false;
	}

	return ret;
}

Resource* M_ResourceManager::AllocateResource(const uint32& uid, const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not allocate Resource in Memory! Error: Given Path was nullptr.");
		return nullptr;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not allocate Resource {" + std::string(assetsPath) + "} in Memory";

	if (uid == 0)
	{
		LOG("%s! Error: Invalid UID.", errorString.c_str());
		return nullptr;
	}
	if (library.find(uid) == library.end())
	{
		LOG("%s! Error: Resource could not be found in Library.", errorString.c_str());
		return nullptr;
	}

	auto item = resources.find(uid);
	if (item != resources.end())
	{
		return item->second;
	}

	char* buffer				= nullptr;
	const char* libraryPath	= library.find(uid)->second.c_str();
	uint read					= App->fileSystem->Load(libraryPath, &buffer);
	if (read == 0)
	{
		LOG("%s! Error: File system could not read File [%s]", errorString.c_str(), libraryPath);
		return nullptr;
	}

	ResourceType type	= GetTypeFromLibraryExtension(libraryPath);
	Resource* resource	= CreateResource(type, assetsPath, uid);
	bool success		= false;
	switch (type)
	{
	case ResourceType::MODEL:		{ success = Importer::Scenes::Load(buffer, (R_Model*)resource); }				break;
	case ResourceType::MESH:		{ success = Importer::Meshes::Load(buffer, (R_Mesh*)resource); }				break;
	case ResourceType::MATERIAL:	{ success = Importer::Materials::Load(buffer, (R_Material*)resource); }			break;
	case ResourceType::TEXTURE:	{ success = Importer::Textures::Load(buffer, read, (R_Texture*)resource); }		break;
	case ResourceType::FOLDER:		{ success = Importer::Folders::Load(buffer, (R_Folder*)resource); }				break;
	case ResourceType::SCENE:		{ /*success = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }			break;
	case ResourceType::ANIMATION:	{ success = Importer::Animations::Load(buffer, (R_Animation*)resource); }		break;
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
		LOG("%s! Error: Importer could not load the Resource Data from File [%s].", errorString.c_str(), libraryPath);
	}

	return resource;
}

bool M_ResourceManager::DeallocateResource(const uint32& uid)
{
	bool ret = true;
	
	if (uid == 0)
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource! Error: Given UID was 0");
		return false;
	}

	RESOURCE_ITEM item = resources.find(uid);
	if (item != resources.end())
	{
		item->second->CleanUp();
		RELEASE(item->second);
		resources.erase(uid);
		return true;
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource %lu! Error: Resource was not allocated in memory.", uid);
	}

	return ret;
}

bool M_ResourceManager::DeallocateResource(Resource* resourceToDeallocate)
{
	bool ret = true;

	if (resourceToDeallocate == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not deallocate Resource! Error: Given Resource* was nullptr");
		return false;
	}

	uint32 resourceUid = resourceToDeallocate->GetUID();

	resourceToDeallocate->CleanUp();
	RELEASE(resourceToDeallocate);

	if (resources.find(resourceUid) != resources.end())
	{
		resources.erase(resourceUid);
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