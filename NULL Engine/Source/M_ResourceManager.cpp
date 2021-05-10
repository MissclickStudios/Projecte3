#include "VariableTypedefs.h"
#include "JSONParser.h"
#include "Profiler.h"
#include "MC_Time.h"
#include "Random.h"
#include "GameObject.h"
//#include "Prefab.h"

#include <time.h>

#include "Importer.h"
#include "I_Scenes.h"
#include "I_Meshes.h"
#include "I_Materials.h"
#include "I_Textures.h"
#include "I_Folders.h"
#include "I_Animations.h"
#include "I_Shaders.h"
#include "I_Particles.h"
#include "I_Scripts.h"
#include "I_Navigation.h"

#include "Application.h"
#include "FileSystemDefinitions.h"
#include "M_FileSystem.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "C_Transform.h"

#include "Resource.h"
#include "R_Mesh.h"
#include "R_Material.h"
#include "R_Texture.h"
#include "R_Model.h"
#include "R_Folder.h"
#include "R_Scene.h"
#include "R_Animation.h"
#include "R_Shader.h"
#include "R_ParticleSystem.h"
#include "R_Script.h"
#include "R_NavMesh.h"

#include "M_ResourceManager.h"

#include "MemoryManager.h"

M_ResourceManager::M_ResourceManager() : Module("ResourceManager"),
fileRefreshTimer	(0.0f),
fileRefreshRate		(0.0f)
{

}

M_ResourceManager::~M_ResourceManager()
{

}
 
bool M_ResourceManager::Init(ParsonNode& configuration)
{
	//file_refresh_rate = (float)configuration.GetNumber("RefreshRate");
	fileRefreshRate = 5.0f;

	return true;
}

bool M_ResourceManager::Start()
{
	RefreshDirectoryFiles(ASSETS_DIRECTORY);
	RefreshDirectoryFiles(ENGINE_DIRECTORY);

	FindPrefabs();

	return true;
}

UpdateStatus M_ResourceManager::PreUpdate(float dt)
{
	OPTICK_CATEGORY("M_ResourceManager PreUpdate", Optick::Category::Module);

	UpdateStatus status = UpdateStatus::CONTINUE;

	fileRefreshTimer += MC_Time::Real::GetDT();

	if (fileRefreshTimer > fileRefreshRate)
	{
		auto item = resources.begin();
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
	return UpdateStatus::CONTINUE;
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
// --- IMPORT FILE METHODS--
uint32 M_ResourceManager::ImportFile(const char* assetsPath)
{
	uint32 resourceUid = 0;

	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: Path was nullptr.");
		return 0;
	}
	if (HasImportIgnoredExtension(assetsPath))																								// TMP until R_Scene has been fully implemented.
	{
		LOG("[ERROR] Resource Manager: Could not Import File from the given path! Error: File to Import had an Ignored Extension");
		return 0;
	}

	assetsPath			= GetValidPath(assetsPath);
	bool metaIsValid	= MetaFileIsValid(assetsPath);
	if (!metaIsValid)
	{
		if (HasMetaFile(assetsPath))
		{
			DeleteFromLibrary(assetsPath);																									// Cleaning any remaining Library files.
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

		std::map<uint32, ResourceBase> libraryItems;
		GetLibraryPairsFromMeta(assetsPath, libraryItems);

		for (auto item = libraryItems.begin(); item != libraryItems.end(); ++item)
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

uint M_ResourceManager::SaveResourceToLibrary(Resource* resource)
{	
	if (resource == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Save Resource to Library! Error: Resource* was nullptr.");
		return 0;
	}

	uint written = 0;
	char* buffer = nullptr;

	switch (resource->GetType())
	{
	case ResourceType::MODEL:			{ written = Importer::Scenes::Save((R_Model*)resource, &buffer); }				break;
	case ResourceType::MESH:			{ written = Importer::Meshes::Save((R_Mesh*)resource, &buffer); }				break;
	case ResourceType::MATERIAL:		{ written = Importer::Materials::Save((R_Material*)resource, &buffer); }		break;
	case ResourceType::TEXTURE:			{ written = Importer::Textures::Save((R_Texture*)resource, &buffer); }			break;
	case ResourceType::FOLDER:			{ written = Importer::Folders::Save((R_Folder*)resource, &buffer); }			break;
	case ResourceType::SCENE:			{ /*written = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }			break;
	case ResourceType::ANIMATION:		{ written = Importer::Animations::Save((R_Animation*)resource, &buffer); }		break;
	case ResourceType::SHADER:			{ written = Importer::Shaders::Save((R_Shader*)resource, &buffer); }			break;
	case ResourceType::PARTICLE_SYSTEM:	{ written = Importer::Particles::Save((R_ParticleSystem*)resource, &buffer); }	break;
	case ResourceType::SCRIPT:			{ written = Importer::Scripts::Save((R_Script*)resource, &buffer); }			break;
	case ResourceType::NAVMESH:	{ written = Importer::Navigation::Save((R_NavMesh*)resource, &buffer); }		break;
	}

	RELEASE_ARRAY(buffer);

	if (written == 0)
	{
		LOG("[ERROR] Resource Manager: Could not save Resource in the Library! Error: Check for Importer Errors in the Console Panel.");
		return 0;
	}

	if (ResourceHasMetaType(resource) /*&& !resource->hasForcedUID*/)
	{
		SaveMetaFile(resource);
	}

	library.emplace(resource->GetUID(), ResourceBase(resource));

	return written;
}

uint32 M_ResourceManager::LoadFromLibrary(const char* assetsPath)
{	
	std::string errorString = "[ERROR] Resource Manager: Could not Load File from the given Library Path";

	assetsPath = GetValidPath(assetsPath);
	if (assetsPath == nullptr)
	{
		LOG("%s! Error: Path was nullptr.", errorString.c_str());
		return 0;
	}

	char* buffer			= nullptr;
	ParsonNode metaRoot		= LoadMetaFile(assetsPath, &buffer);
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

	uint32 resourceUid			= (uint32)metaRoot.GetNumber("UID");
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
	
	if (resources.find(resourceUid) != resources.end())
	{
		return resourceUid;																									// If the File To Load's Resource is already in memory.
	}	

	bool success = AllocateResource(resourceUid, assetsPath);
	if (!success)
	{
		LOG("[ERROR] Resource Manager: Could not Allocate Resource %lu in memory!", resourceUid);
		return 0;
	}

	std::string containedPath = "";
	std::string containedName = "";
	for (uint i = 0; i < containedArray.size; ++i)
	{
		ParsonNode containedNode = containedArray.GetNode(i);

		App->fileSystem->SplitFilePath(assetsPath, &containedPath, nullptr, nullptr);										// --- TMP Until Something Functional Is In Place.
		containedName = containedNode.GetString("Name");																	// 
		containedPath += containedName;																						// -----------------------------------------------

		uint32 containedUid = (uint32)containedNode.GetNumber("UID");

		if (resources.find(containedUid) != resources.end())																// No need to allocate if it is already allocated.
		{
			continue;
		}

		success = AllocateResource(containedUid, containedPath.c_str());
		if (!success)
		{
			LOG("[WARNING] Resource Manager: Could not allocate Contained Resource! UID: %lu, Name: %s", containedUid, containedName.c_str());
		}

		containedName.clear();
		containedPath.clear();
	}

	return resourceUid;
}

Resource* M_ResourceManager::GetResourceFromLibrary(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource associated with Meta File! Error: Given Assets Path was nullptr.");
		return nullptr;
	}

	std::string metaFile = assetsPath + std::string(META_EXTENSION);
	std::string errorString = "[ERROR] Resource Manager: Could not get Resource associated with { " + metaFile + " } Meta File";

	if (!App->fileSystem->Exists(metaFile.c_str()))
	{
		LOG("%s! Error: File System could not find Meta File.", errorString.c_str());
		return nullptr;
	}

	uint32 resourceUid = LoadFromLibrary(assetsPath);
	if (resourceUid == 0)
	{
		LOG("%s! Error: Could not get Resource UID from Assets Path.", errorString.c_str());
		return nullptr;
	}

	Resource* resource = RequestResource(resourceUid);
	if (resource == nullptr)
	{
		LOG("%s! Error: Could not Request Resource.", errorString.c_str());
	}

	return resource;
}

void M_ResourceManager::RefreshProjectDirectories()
{
	RefreshDirectoryFiles(ASSETS_DIRECTORY);
	RefreshDirectoryFiles(ENGINE_DIRECTORY);
}

void M_ResourceManager::RefreshProjectDirectory(const char* directoryToRefresh)
{
	RefreshDirectoryFiles(directoryToRefresh);
}

ResourceType M_ResourceManager::GetTypeFromAssetsExtension(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get resource type from assets extension! Error: Given Assets Path String was nullptr.");
		return ResourceType::NONE;
	}
	
	ResourceType type		= ResourceType::NONE;
	std::string extension	= App->fileSystem->GetFileExtension(assetsPath);

	if (extension == "fbx" || extension == "FBX" || extension == "obj" || extension == "OBJ")
	{
		type = ResourceType::MODEL;
	}
	else if (extension == "png" || extension == "PNG" || extension == "tga" || extension == "TGA" || extension == "dds" || extension == "DDS")
	{
		type = ResourceType::TEXTURE;
	}
	else if (extension == "json" || extension == "JSON")
	{
		type = ResourceType::SCENE;
	}
	else if (extension == "shader")
	{
		type = ResourceType::SHADER;
	}
	else if (extension == "prefab")
	{
		type = ResourceType::PREFAB;
	}
	else if (extension == "particles")
	{
		type = ResourceType::PARTICLE_SYSTEM;
	}
	else if (extension == "h")
	{
		type = ResourceType::SCRIPT;
	}
	else if (extension == "navmesh")
	{
		type = ResourceType::NAVMESH;
	}
	else if (extension == "[NONE]")
	{
		type = ResourceType::FOLDER;
	}
	else
	{
		LOG("[ERROR] Resource Manager: Could not import from the given Assets Path! Error: File extension { %s } is not supported!", extension.c_str());
	}

	return type;
}

uint32 M_ResourceManager::GetForcedUIDFromMeta(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Forced UIDs from Meta! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string errorString		= "[ERROR] Resource Manager: Could not get Forced UIDs from { " + std::string(assetsPath) + " }'s Meta File.";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no correspondent Meta File.", errorString.c_str());
		return 0;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: Contained Array in Meta File was not valid.", errorString.c_str());
		return 0;
	}

	return ((uint32)metaRoot.GetNumber("UID"));
}

bool M_ResourceManager::GetForcedUIDsFromMeta(const char* assetsPath, std::map<std::string, uint32>& forcedUIDs)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Forced UIDs from Meta! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string errorString		= "[ERROR] Resource Manager: Could not get Forced UIDs from { " + std::string(assetsPath) + " }'s Meta File.";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
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
	std::string assetName	= metaRoot.GetString("Name");
	uint32 resourceUID		= (uint32)metaRoot.GetNumber("UID");
	if (resourceUID == 0)
	{
		LOG("%s! Error: Main ResourceUID was 0.", errorString.c_str());
		return false;
	}

	forcedUIDs.emplace(assetName, resourceUID);

	// --- CONTAINED RESOURCES
	uint32 containedUID			= 0;
	std::string containedName	= "[NONE]";
	ParsonNode containedNode	= ParsonNode();
	for (uint i = 0; i < containedArray.size; ++i)
	{
		containedNode	= containedArray.GetNode(i);
		if (!containedNode.NodeIsValid())
		{
			continue;
		}

		containedName	= containedNode.GetString("Name");
		containedUID	= (uint32)containedNode.GetNumber("UID");
		if (containedUID == 0)
		{
			continue;
		}

		forcedUIDs.emplace(containedName, containedUID);
	}

	return true;
}

// --- RESOURCE METHODS ---
Resource* M_ResourceManager::CreateResource(ResourceType type, const char* assetsPath, uint32 forcedUID)
{
	Resource* resource = nullptr;
	switch (type)
	{
	case ResourceType::MESH:			{ resource = new R_Mesh(); }			break;
	case ResourceType::MATERIAL:		{ resource = new R_Material(); }		break;
	case ResourceType::TEXTURE:			{ resource = new R_Texture(); }			break;
	case ResourceType::MODEL:			{ resource = new R_Model(); }			break;
	case ResourceType::FOLDER:			{ resource = new R_Folder(); }			break;
	case ResourceType::SCENE:			{ resource = new R_Scene(); }			break;
	case ResourceType::ANIMATION:		{ resource = new R_Animation(); }		break;
	case ResourceType::SHADER:			{ resource = new R_Shader(); }			break;
	case ResourceType::PARTICLE_SYSTEM:	{ resource = new R_ParticleSystem(); }	break;
	case ResourceType::SCRIPT:			{ resource = new R_Script(); }			break;
	case ResourceType::NAVMESH:			{ resource = new R_NavMesh(); }			break;
	case ResourceType::NONE:			{ /*resource = nullptr;*/ }				break;											// In case NONE is a trigger and a method needs to be called.
	}

	if (resource != nullptr)
	{
		if (assetsPath != nullptr)
		{
			resource->SetAssetsPathAndFile(assetsPath, App->fileSystem->GetFileAndExtension(assetsPath).c_str());
		}

		(forcedUID != 0) ? resource->ForceUID(forcedUID) : resource->SetLibraryPathAndFile();
	}

	return resource;
}

Resource* M_ResourceManager::RequestResource(uint32 UID)
{	
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Resource Request Failed! Error: Requested UID was 0.");
		return nullptr;
	}
	
	Resource* resource	= nullptr;
	auto rItem			= resources.find(UID);
	if (rItem != resources.end())
	{
		rItem->second->ModifyReferences(1);
		resource = rItem->second;
	}

	return resource;
}

bool M_ResourceManager::AllocateResource(uint32 UID, const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not allocate Resource in Memory! Error: Given Path was nullptr.");
		return false;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not allocate Resource {" + std::string(assetsPath) + "} in Memory";

	if (UID == 0)
	{
		LOG("%s! Error: Invalid UID.", errorString.c_str());
		return false;
	}
	if (library.find(UID) == library.end())
	{
		LOG("%s! Error: Resource could not be found in Library.", errorString.c_str());
		return false;
	}

	auto item = resources.find(UID);
	if (item != resources.end())
	{
		return true;
	}

	char* buffer				= nullptr;
	const char* libraryPath		= library.find(UID)->second.libraryPath.c_str();
	uint read					= App->fileSystem->Load(libraryPath, &buffer);
	if (read == 0)
	{
		LOG("%s! Error: File system could not read File [%s]", errorString.c_str(), libraryPath);
		return false;
	}

	ResourceType type	= GetTypeFromLibraryExtension(libraryPath);
	Resource* resource	= CreateResource(type, assetsPath, UID);
	bool success		= false;
	switch (type)
	{
	case ResourceType::MODEL:			{ success = Importer::Scenes::Load(buffer, (R_Model*)resource); }				break;
	case ResourceType::MESH:			{ success = Importer::Meshes::Load(buffer, (R_Mesh*)resource); }				break;
	case ResourceType::MATERIAL:		{ success = Importer::Materials::Load(buffer, (R_Material*)resource); }			break;
	case ResourceType::TEXTURE:			{ success = Importer::Textures::Load(buffer, read, (R_Texture*)resource); }		break;
	case ResourceType::FOLDER:			{ success = Importer::Folders::Load(buffer, (R_Folder*)resource); }				break;
	case ResourceType::SCENE:			{ /*success = TODO: HAVE A FUNCTIONAL R_SCENE AND SAVE/LOAD METHODS*/ }			break;
	case ResourceType::ANIMATION:		{ success = Importer::Animations::Load(buffer, (R_Animation*)resource); }		break;
	case ResourceType::SHADER:			{ success = Importer::Shaders::Load(buffer, (R_Shader*)resource); }				break;
	case ResourceType::PARTICLE_SYSTEM:	{ success = Importer::Particles::Load(buffer, (R_ParticleSystem*)resource); }	break;
	case ResourceType::SCRIPT:			{ success = Importer::Scripts::Load(buffer, (R_Script*)resource); }				break;
	case ResourceType::NAVMESH:			{ success = Importer::Navigation::Load(buffer, (R_NavMesh*)resource); }			break;
	case ResourceType::NONE:			{ /*success = false;*/ }														break;	// In case NONE is a trigger and a method needs to be called.
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

	return success;
}

bool M_ResourceManager::FreeResource(uint32 UID)
{	
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Freed UID was 0.");
		return false;
	}
	
	auto rItem = resources.find(UID);
	if (rItem == resources.end())
	{
		LOG("[ERROR] Resource Manager: Free Resource Operation Failed! Error: Given UID could not be found in Resources Map.");
		return false;
	}

	rItem->second->ModifyReferences(-1);
	if (rItem->second->GetReferences() == 0)
	{
		DeallocateResource(UID);
	}

	return true;
}

bool M_ResourceManager::DeallocateResource(uint32 UID)
{
	if (UID == 0)
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource! Error: Given UID was 0");
		return false;
	}

	auto item = resources.find(UID);
	if (item == resources.end())
	{
		LOG("[ERROR] Resource Manager: Could not Deallocate Resource %lu! Error: Resource was not allocated in memory.", UID);
		return false;
	}

	item->second->CleanUp();
	RELEASE(item->second);
	resources.erase(UID);

	return true;
}

bool M_ResourceManager::DeallocateResource(Resource* resourceToDeallocate)
{
	if (resourceToDeallocate == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not deallocate Resource! Error: Given Resource* was nullptr.");
		return false;
	}

	uint32 resourceUID = resourceToDeallocate->GetUID();

	resourceToDeallocate->CleanUp();
	RELEASE(resourceToDeallocate);

	(resources.find(resourceUID) != resources.end()) ? resources.erase(resourceUID) : LOG("[WARNING] Resource Manager: Deallocated resource was not stored in resources map!");

	return true;
}

bool M_ResourceManager::DeleteResource(uint32 UID)
{
	if (library.find(UID) != library.end())
	{
		library.erase(UID);
	}

	auto item = resources.find(UID);
	if(item == resources.end())
	{
		LOG("[ERROR] Resource Manager: Could not delete Resource! Error: Resource to delete { %lu } was not inside the resources map!", UID);
		return false;
	}

	if (item->second != nullptr)																					// item->second is the Resource* to delete.
	{
		item->second->CleanUp();
		RELEASE(item->second);
	}

	resources.erase(UID);

	return true;
}

bool M_ResourceManager::DeleteResource(Resource* resourceToDelete)
{
	if (resourceToDelete == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not delete given Resource! Error: Given Resource* was nullptr.");
		return false;
	}
	
	uint32 resourceUID = resourceToDelete->GetUID();

	resourceToDelete->CleanUp();
	RELEASE(resourceToDelete);

	//(library.find(resourceUID) != library.end()) ? library.erase(resourceUID) : LOG("[WARNING] ResourceManager: Deleted Resources was not stored in library map!");		// TMP. Revise
	
	(resources.find(resourceUID) != resources.end()) ? resources.erase(resourceUID) : LOG("[WARNING] Resource Manager: Deleted Resource was not stored in resources map!");

	return true;
}

bool M_ResourceManager::RefreshResourceAssets(ResourceType type)
{
	/*if (!IsMetaType(type))
		return;*/
	
	switch (type)
	{
	case ResourceType::MODEL:			{ RefreshDirectoryFiles(ASSETS_MODELS_PATH); }			break;
	case ResourceType::TEXTURE:			{ RefreshDirectoryFiles(ASSETS_TEXTURES_PATH); }		break;
	case ResourceType::FOLDER:			{ /*RefreshDirectoryFiles(ASSETS_FOLDERS_PATH);*/ }		break;
	case ResourceType::SCENE:			{ /*RefreshDirectoryFiles(ASSETS_SCENES_PATH);*/ }		break;
	case ResourceType::SHADER:			{ RefreshDirectoryFiles(ASSETS_SHADERS_PATH); }			break;
	case ResourceType::PARTICLE_SYSTEM:	{ RefreshDirectoryFiles(ASSETS_PARTICLESYSTEMS_PATH); }	break;
	case ResourceType::SCRIPT:			{ RefreshDirectoryFiles(ASSETS_SCRIPTS_PATH); }			break;
	case ResourceType::NAVMESH:			{ RefreshDirectoryFiles(ASSETS_NAVIGATION_PATH); }		break;
	case ResourceType::NONE:			{ RefreshDirectoryFiles(ASSETS_PATH); }					break;
	}

	return true;
}

const std::map<uint32, Resource*>* M_ResourceManager::GetResourcesMap() const
{
	return &this->resources;
}

R_Shader* M_ResourceManager::GetShader(const char* name)
{
	if (name == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Get Shader! Error: Given name string was nullptr.");
		return nullptr;
	}
	
	std::string defaultPath = ASSETS_SHADERS_PATH + std::string(name) + ".shader"; 
	R_Shader* tempShader	= (R_Shader*)App->resourceManager->GetResourceFromLibrary(defaultPath.c_str());

	if (tempShader == nullptr)
	{
		LOG("[ERROR] Could not Get Shader! Error: Shader { %s } could not be found in active resources.", name);
		return nullptr;
	}

	return tempShader;
}

void M_ResourceManager::GetAllShaders(std::vector<R_Shader*>& shaders)
{
	std::vector<std::string> shdrFiles;
	App->fileSystem->GetAllFilesWithFilter(ASSETS_SHADERS_PATH, shdrFiles, nullptr, "shader");
	if (shdrFiles.empty())
		return;

	R_Shader* tmpShader = nullptr;
	for (uint i = 0; i < shdrFiles.size(); i++)
	{
		//std::string defaultPath = ASSETS_SHADERS_PATH + std::string(shaderFiles[i]) + SHADERS_EXTENSION;
		tmpShader = (R_Shader*)App->resourceManager->GetResourceFromLibrary(shdrFiles[i].c_str());
		(tmpShader != nullptr) ? shaders.push_back(tmpShader) : LOG("[ERROR] Could not get Shader! Error: Shader { %s } could not be found in active resources.", shdrFiles[i], shdrFiles[i]);
	}

	tmpShader = nullptr;
	shdrFiles.clear();
}

void M_ResourceManager::GetAllParticleSystems(std::vector<R_ParticleSystem*>& shaders)
{
	std::vector<std::string> particleSystemFiles;
	App->fileSystem->GetAllFilesWithFilter(ASSETS_PARTICLESYSTEMS_PATH, particleSystemFiles, nullptr,"particles");
	if (particleSystemFiles.empty())
		return;

	R_ParticleSystem* tmpParticleSystem = nullptr;
	for (uint i = 0; i < particleSystemFiles.size(); i++)
	{
		tmpParticleSystem = (R_ParticleSystem*)App->resourceManager->GetResourceFromLibrary(particleSystemFiles[i].c_str());
		(tmpParticleSystem != nullptr) ? shaders.push_back(tmpParticleSystem) : LOG("[ERROR] Could not get Shader! Error: ParticleSystem { %s } could not be found in active resources.", particleSystemFiles[i], particleSystemFiles[i]);
	}

	tmpParticleSystem = nullptr;
	particleSystemFiles.clear();
}

void M_ResourceManager::GetAllTextures(std::vector<R_Texture*>& textures, const char* name)
{
	std::vector<std::string> texFiles;
	std::vector<std::string> nameFilters;
	std::vector<std::string> extFilters;
	extFilters.push_back("png");
	extFilters.push_back("tga");
	extFilters.push_back("dds");
	if (name != nullptr)  nameFilters.push_back(name);
	
	App->fileSystem->GetAllFilesWithFilters(ASSETS_TEXTURES_PATH, texFiles, nameFilters, extFilters);
	if (texFiles.empty())
	{
		extFilters.clear();
		return;
	}

	R_Texture* tempTex = nullptr;
	for (uint i = 0; i < texFiles.size(); i++)
	{
		tempTex = (R_Texture*)App->resourceManager->GetResourceFromLibrary(texFiles[i].c_str());
		(tempTex != nullptr) ? textures.push_back(tempTex) : LOG("[ERROR] Could not get Texture! Error: Texture { %s } could not be found in active resources.", texFiles[i], texFiles[i]);
	}

	tempTex = nullptr;
	
	texFiles.clear();
	nameFilters.clear();
	extFilters.clear();
}

void M_ResourceManager::GetAllScripts(std::map<std::string, std::string>& scripts)
{
	std::vector<std::string> scriptFiles;
	App->fileSystem->GetAllFilesWithFilter(ASSETS_SCRIPTS_PATH, scriptFiles, nullptr, "h");
	if (scriptFiles.empty())
		return;
	
	R_Script* tempScript = nullptr;
	for (uint i = 0; i < scriptFiles.size(); i++)
	{
		tempScript = (R_Script*)App->resourceManager->GetResourceFromLibrary(scriptFiles[i].c_str());
		if (tempScript == nullptr)
		{
			LOG("[ERROR] Could not get the %s Error: %s could not be found in active resources.", scriptFiles[i].c_str(), scriptFiles[i].c_str());
			continue;
		}
		else
		{
			for (int z = 0; z < tempScript->dataStructures.size(); ++z)
			{
				scripts.emplace(tempScript->dataStructures[z].first, scriptFiles[i]);
			}

			App->resourceManager->FreeResource(tempScript->GetUID());
		}
	}

	tempScript = nullptr;
	scriptFiles.clear();
}

void M_ResourceManager::ReloadAllScripts()
{
	RefreshDirectoryFiles(ASSETS_SCRIPTS_PATH);
}

// --- PREFAB METHODS
void M_ResourceManager::CreatePrefab(GameObject* gameObject)
{
	uint id = Random::LCG::GetRandomUint();
	gameObject->SetAsPrefab(id);

	SavePrefab(gameObject, id);

	prefabs.emplace(id, Prefab(id, gameObject->GetName(), MC_Time::GetUnixTime()));
}

void M_ResourceManager::UpdatePrefab(GameObject* gameObject)
{
	GameObject* tmp = gameObject;
	GameObject* parent = gameObject->parent;

	while (parent != nullptr && parent->prefabID == gameObject->prefabID) //Find parent
	{
		tmp = parent;
		parent = tmp->parent;
	}

	uint id = gameObject->prefabID;
	tmp->SetAsPrefab(id);
	SavePrefab(tmp, id);

	std::map<uint32,Prefab>::iterator prefab = prefabs.find(id);
	
	if(prefab != prefabs.end())
		prefab->second.updateTime = MC_Time::GetUnixTime();
}

void M_ResourceManager::SavePrefab(GameObject* gameObject, uint _prefabId)
{
	//Always reorder ui before saving prefabs or scenes
	App->uiSystem->SaveCanvasChildrenOrder();
	ParsonNode rootNode;

	SavePrefabObject(gameObject,&rootNode);

	char* buffer = nullptr;
	std::string fileName = ASSETS_PREFABS_PATH + std::to_string(_prefabId) + PREFAB_EXTENSION;
	rootNode.SerializeToFile(fileName.c_str(),&buffer);

	RELEASE_ARRAY(buffer);
}

void M_ResourceManager::SavePrefabObject(GameObject* gameObject, ParsonNode* node)
{
	gameObject->SaveState(*node);

	ParsonArray childsArray = node->SetArray("Children");

	for (auto child = gameObject->childs.begin(); child != gameObject->childs.end(); child++)
	{
		ParsonNode childNode = childsArray.SetNode("child");
		SavePrefabObject((*child), &childNode);
	}
}

GameObject* M_ResourceManager::LoadPrefab(uint _prefabId, GameObject* parent, GameObject* rootObject)
{
	char* buffer = nullptr;
	std::string fileName = ASSETS_PREFABS_PATH + std::to_string(_prefabId) + PREFAB_EXTENSION;
	uint f = App->fileSystem->Load(fileName.c_str(), &buffer);
	if (f == 0)
	{
		LOG("Could not load prefab with ID: %d into scene", _prefabId);
		return nullptr;
	}

	ParsonNode prefabRoot(buffer);
	RELEASE_ARRAY(buffer);

	GameObject* rootObjectLoaded = App->scene->LoadPrefabIntoScene(&prefabRoot,parent);

	if(rootObject != nullptr) //we use the transform from the root object to keep it in the same place it was in the scene
	{
		rootObjectLoaded->transform->SetLocalTransform(rootObject->transform->GetLocalTransform());
		rootObjectLoaded->ForceUID(rootObject->GetUID());
	}

	return rootObjectLoaded;
}

Prefab* M_ResourceManager::GetPrefab(uint uid)
{
	std::map<uint, Prefab>::iterator prefab = prefabs.find(uid);
	if (prefab != prefabs.end())
	{
		return &prefab->second;
	}
	return nullptr;
}

const char* M_ResourceManager::GetPrefabName(uint uid)
{
	std::map<uint, Prefab>::iterator prefab = prefabs.find(uid);
	if (prefab != prefabs.end())
	{
		return prefab->second.name.c_str();
	}
	return nullptr;
}

Prefab* M_ResourceManager::GetPrefabByName(const char* prefabName)
{
	for (auto prefab = prefabs.begin(); prefab != prefabs.end(); ++prefab)
		if (strcmp(prefab->second.name.c_str(), prefabName) == 0)
			return &prefab->second;
	return nullptr;
}

uint M_ResourceManager::GetPrefabUIDByName(const char* prefabName)
{
	for (auto prefab = prefabs.begin(); prefab != prefabs.end(); ++prefab)
		if (strcmp(prefab->second.name.c_str(), prefabName) == 0)
			return prefab->second.uid;
	return 0;
}

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
	
	std::vector<std::string> assetFiles;
	std::vector<std::string> metaFiles;
	std::map<std::string, std::string> filePairs;

	App->fileSystem->DiscoverAllFilesFiltered(directory, assetFiles, metaFiles, DOTLESS_META_EXTENSION);

	FindFilesToImport(assetFiles, metaFiles, filePairs, filesToImport);											// Always call in this order!
	FindFilesToUpdate(filePairs, filesToUpdate);																// At the very least FindFilesToImport() has to be the first to be called
	FindFilesToDelete(metaFiles, filePairs, filesToDelete);														// as it is the one to fill file_pairs with asset and meta files!

	LoadValidFilesIntoLibrary(filePairs);																		// Will emplace all valid files' UID & library path into the library map.

	filePairs.clear();
	metaFiles.clear();
	assetFiles.clear();
}

void M_ResourceManager::FindFilesToImport(const std::vector<std::string>& assetFiles, const std::vector<std::string>& metaFiles, 
											std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToImport)
{
	if (assetFiles.empty())
		return;
	
	std::map<std::string, uint> metaTmp;
	for (uint i = 0; i < metaFiles.size(); ++i)																	// Adding the meta_files to a map so the elements can be found with ease.
	{
		metaTmp.emplace(metaFiles[i], i);
	}

	std::string metaFile = "[NONE]";
	std::map<std::string, uint>::iterator item;
	for (uint i = 0; i < assetFiles.size(); ++i)																// Assets files whose meta file is missing will be imported.
	{
		if (HasImportIgnoredExtension(assetFiles[i].c_str()))													// TMP. Maybe prefabs will have a meta and a custom file format later on.
		{
			continue;
		}
		
		metaFile	= assetFiles[i] + META_EXTENSION;
		item		= metaTmp.find(metaFile);

		if (item != metaTmp.end() /*&& MetaFileIsValid(asset_files[i].c_str())*/)
		{
			filePairs.emplace(assetFiles[i], item->first);

			if (!MetaFileIsValid(assetFiles[i].c_str(), false))													// In case the pair exists but the meta file is outdated.
			{
				filesToImport.push_back(assetFiles[i]);															// SHARING SCENES: STEP 1
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
	if (filePairs.empty())
		return;
	
	uint64 assetModTime	= 0;
	uint64 metaModTime	= 0;
	for (auto item = filePairs.begin(); item != filePairs.end(); ++item)
	{
		assetModTime	= App->fileSystem->GetLastModTime(item->first.c_str());							// Files with different modification time than their meta files
		metaModTime		= GetAssetFileModTimeFromMeta(item->first.c_str());								// will need to be updated (Delete prev + Import new).

		if (assetModTime != metaModTime)
		{
			LOG("[WARNING] Resource Manager: File Modification Time discrepancy! File: { %s } ModTimes: [%llu] :: [%llu]", item->first.c_str(), assetModTime, metaModTime);
			filesToUpdate.push_back(item->first);
		}
	}
}

void M_ResourceManager::FindFilesToDelete(const std::vector<std::string>& metaFiles, const std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToDelete)
{
	if (metaFiles.empty())
		return;
	
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
	if (filePairs.empty())
		return;
	
	for (auto item = filePairs.cbegin(); item != filePairs.cend(); ++item)
	{
		LoadMetaLibraryPairsIntoLibrary(item->first.c_str());
	}
}

bool M_ResourceManager::DeleteFromAssets(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Delete File from Assets! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::vector<uint32> resourceUIDs;
	std::vector<std::string> filesToDelete;

	GetResourceUIDsFromMeta(assetsPath, resourceUIDs);
	GetLibraryFilePathsFromMeta(assetsPath, filesToDelete);
	
	std::string metaPath = assetsPath + std::string(META_EXTENSION);
	filesToDelete.push_back(assetsPath);
	filesToDelete.push_back(metaPath);

	for (uint i = 0; i < resourceUIDs.size(); ++i)
	{
		DeleteResource(resourceUIDs[i]);
	}

	for (uint i = 0; i < filesToDelete.size(); ++i)
	{
		/*ret =*/ App->fileSystem->Remove(filesToDelete[i].c_str());
	}

	filesToDelete.clear();
	resourceUIDs.clear();

	return true;
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
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Resource UIDs from Meta! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not get Resource UIDs from { " + std::string(assetsPath) + " }'s Meta";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
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
	uint32 containedUid			= 0;
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

	return true;
}

bool M_ResourceManager::GetLibraryFilePathsFromMeta(const char* assetsPath, std::vector<std::string>& filePaths)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}
	
	std::string errorString = "[ERROR] Resoruce Manager: Could not get Library File Paths from { " + std::string(assetsPath) + " }'s Meta File";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
		return false;
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
	ResourceType type		= (ResourceType)((int)metaRoot.GetNumber("Type"));
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
	ParsonNode containedNode	= ParsonNode();
	uint32 containedUid			= 0;
	ResourceType containedType	= ResourceType::NONE;
	std::string containedPath	= "[NONE]";
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
		containedType	= (ResourceType)((int)containedNode.GetNumber("Type"));

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

	return true;
}

bool M_ResourceManager::GetResourceBasesFromMeta(const char* assetsPath, std::vector<ResourceBase>& resourceBases)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}
	
	std::string errorString = "[ERROR] Resoruce Manager: Could not get Library File Paths from { " + std::string(assetsPath) + " }'s Meta File";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
	RELEASE_ARRAY(buffer);

	if (!metaRoot.NodeIsValid())
	{
		LOG("%s! Error: Given Assets Path had no associated .meta file.", errorString.c_str());
		return false;
	}
	if (!containedArray.ArrayIsValid())
	{
		LOG("%s! Error: ContainedResources array in Meta File was not valid.", errorString.c_str());
		return false;
	}

	// --- MAIN RESOURCE
	uint32 UID					= (uint32)metaRoot.GetNumber("UID");
	ResourceType type			= (ResourceType)((int)metaRoot.GetNumber("Type"));
	std::string rAssetsPath		= metaRoot.GetString("AssetsPath");
	std::string rAssetsFile		= metaRoot.GetString("Name");
	std::string rLibraryPath	= metaRoot.GetString("LibraryPath");
	std::string rLibraryFile	= metaRoot.GetString("LibraryFile");
	if (UID == 0)
	{
		LOG("%s! Error: Main Resource UID was 0.", errorString.c_str());
		return false;
	}

	resourceBases.push_back(ResourceBase(UID, rAssetsPath, rAssetsFile, rLibraryPath, rLibraryFile, type));

	// --- CONTAINED RESOURCES
	for (uint i = 0; i < containedArray.size; ++i)
	{
		ParsonNode containedNode = containedArray.GetNode(i);
		if (!containedNode.NodeIsValid())
		{
			continue;
		}

		std::string directory = "[NONE]";
		std::string extension = "[NONE]";

		uint32 containedUID					= (uint32)containedNode.GetNumber("UID");
		ResourceType containedType			= (ResourceType)((int)containedNode.GetNumber("Type"));
		std::string containedAssetsPath		= "[NONE]";
		std::string containedAssetsFile		= containedNode.GetString("Name");
		std::string containedLibraryPath	= containedNode.GetString("LibraryPath");
		std::string containedLibraryFile	= "[NONE]";
		bool success						= GetAssetsDirectoryAndExtensionFromType(containedType, directory, extension);
		if (!success)
		{
			continue;
		}
		if (containedUID == 0)
		{
			continue;
		}

		containedAssetsPath		= directory + containedAssetsFile;
		containedLibraryFile	= App->fileSystem->GetFileAndExtension(containedLibraryPath.c_str());
		resourceBases.push_back(ResourceBase(containedUID, containedAssetsPath, containedAssetsFile, containedLibraryPath, containedLibraryFile, containedType));	// WIP until revision.
	}
	
	return true;
}

bool M_ResourceManager::GetAssetsDirectoryAndExtensionFromType(const ResourceType& type, std::string& directory, std::string& extension)				// Extensions are a WIP.
{
	switch (type)
	{
	case ResourceType::MODEL:			{ directory = ASSETS_MODELS_PATH;			/*extension = MODELS_EXTENSION;*/ }				break;
	case ResourceType::MESH:			{ directory = ASSETS_MODELS_PATH;			/*extension = MESHES_EXTENSION;*/ }				break;
	case ResourceType::MATERIAL:		{ directory = ASSETS_MODELS_PATH;			/*extension = MATERIALS_EXTENSION;*/ }			break;
	case ResourceType::TEXTURE:			{ directory = ASSETS_TEXTURES_PATH;			/*extension = TEXTURES_EXTENSION;*/ }			break;
	case ResourceType::FOLDER:			{ /*directory = ASSETS_FOLDERS_PATH;			extension = FOLDERS_EXTENSION;*/ }			break;
	case ResourceType::SCENE:			{ directory = ASSETS_SCENES_PATH;			/*extension = SCENES_EXTENSION;*/ }				break;
	case ResourceType::ANIMATION:		{ directory = ASSETS_MODELS_PATH;			/*extension = ANIMATIONS_EXTENSION;*/ }			break;
	case ResourceType::SHADER:			{ directory = ASSETS_SHADERS_PATH;			/*extension = SHADERS_EXTENSION;*/ }			break;
	case ResourceType::PARTICLE_SYSTEM:	{ directory = ASSETS_PARTICLESYSTEMS_PATH;	/*extension = PARTICLESYSTEMS_EXTENSION;*/ }	break;
	case ResourceType::SCRIPT:			{ directory = ASSETS_SCRIPTS_PATH;			/*extension = SCRIPTS_EXTENSION;*/ }			break;
	case ResourceType::NAVMESH:			{ directory = ASSETS_NAVIGATION_PATH;		/*extension = NAVMESH_EXTENSION;*/ }			break;
	case ResourceType::NONE:			{ return false; }																			break;
	}
	
	return true;
}

bool M_ResourceManager::GetLibraryDirectoryAndExtensionFromType(const ResourceType& type, std::string& directory, std::string& extension)
{	
	switch (type)
	{
	case ResourceType::MODEL:			{ directory = MODELS_PATH;			extension = MODELS_EXTENSION; }				break;
	case ResourceType::MESH:			{ directory = MESHES_PATH;			extension = MESHES_EXTENSION; }				break;
	case ResourceType::MATERIAL:		{ directory = MATERIALS_PATH;		extension = MATERIALS_EXTENSION; }			break;
	case ResourceType::TEXTURE:			{ directory = TEXTURES_PATH;		extension = TEXTURES_EXTENSION; }			break;
	case ResourceType::FOLDER:			{ directory = FOLDERS_PATH;			extension = FOLDERS_EXTENSION; }			break;
	case ResourceType::SCENE:			{ directory = SCENES_PATH;			extension = SCENES_EXTENSION; }				break;
	case ResourceType::ANIMATION:		{ directory = ANIMATIONS_PATH;		extension = ANIMATIONS_EXTENSION; }			break;
	case ResourceType::SHADER:			{ directory = SHADERS_PATH;			extension = SHADERS_EXTENSION; }			break;
	case ResourceType::PARTICLE_SYSTEM:	{ directory = PARTICLESYSTEMS_PATH;	extension = PARTICLESYSTEMS_EXTENSION; }	break;
	case ResourceType::SCRIPT:			{ directory = SCRIPTS_PATH;			extension = SCRIPTS_EXTENSION; }			break;
	case ResourceType::NAVMESH:			{ directory = NAVIGATION_PATH;		extension = NAVMESH_EXTENSION; }			break;
	case ResourceType::NONE:			{ return false; }																break;
	}

	return true;
}

bool M_ResourceManager::LoadMetaLibraryPairsIntoLibrary(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not load Meta Library Pairs into Library! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::map<uint32, ResourceBase> libraryPairs;
	GetLibraryPairsFromMeta(assetsPath, libraryPairs);
	if (libraryPairs.empty())
		return true;

	for (auto item = libraryPairs.begin(); item != libraryPairs.end(); ++item)
	{
		library.emplace(item->first, item->second);
	}

	libraryPairs.clear();

	return true;
}

bool M_ResourceManager::GetLibraryPairsFromMeta(const char* assetsPath, std::map<uint32, ResourceBase>& pairs)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library Pairs from File's associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	std::string errorString = "[ERROR] Resource Manager: Could not get Libarary Pairs from { " + std::string(assetsPath) + " }'s associated Meta File";

	std::vector<uint32> resourceUids;
	std::vector<ResourceBase> resourceBases;

	GetResourceUIDsFromMeta(assetsPath, resourceUids);
	GetResourceBasesFromMeta(assetsPath, resourceBases);

	if (resourceUids.size() != resourceBases.size())
	{
		LOG("%s! Error: Mismatched amount of Resource UIDs and Library Paths.", errorString.c_str());
	}
	for (uint i = 0; i < resourceUids.size(); ++i)
	{
		pairs.emplace(resourceUids[i], resourceBases[i]);
	}

	resourceUids.clear();
	resourceBases.clear();

	return true;
}

uint64 M_ResourceManager::GetAssetFileModTimeFromMeta(const char* assetsPath)
{	
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not get Library File Paths from Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string errorString		= "[ERROR] Resoruce Manager: Could not get Asset File Mod Time from { " + std::string(assetsPath) + " }'s Meta File";

	char* buffer				= nullptr;
	ParsonNode metaRoot			= LoadMetaFile(assetsPath, &buffer);
	ParsonArray containedArray	= metaRoot.GetArray("ContainedResources");
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

	return ((uint64)metaRoot.GetNumber("ModificationTime"));
}

void M_ResourceManager::FindPrefabs()
{
	std::vector<std::string> files,directories;
	App->fileSystem->DiscoverFiles(ASSETS_PREFABS_PATH,files,directories);
	
	char* buffer = nullptr;
	std::string fileName;
	int modTime = 0;
	for (auto file = files.begin(); file != files.end(); file++)
	{
		fileName = ASSETS_PREFABS_PATH + (*file);
		App->fileSystem->Load(fileName.c_str(), &buffer);

		ParsonNode prefab(buffer);
		RELEASE_ARRAY(buffer);

		std::string id;
		App->fileSystem->SplitFilePath((*file).c_str(), nullptr, &id);

		modTime = App->fileSystem->GetLastModTime(fileName.c_str());
		
		prefabs.emplace(atoi((*file).c_str()), Prefab(atoi((*file).c_str()),prefab.GetString("Name"),modTime));												// atoi()? Will there be any problems if you just emplace(a, b)?
	}
}

// --- IMPORT FILE METHODS ---
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
		ResourceType type	= GetTypeFromAssetsExtension(assetsPath);
		Resource* resource	= CreateResource(type, assetsPath);

		bool success = false;
		switch (type)
		{
		case ResourceType::MODEL:			{ success = Importer::ImportScene(buffer, read, (R_Model*)resource); }						break;
		case ResourceType::MESH:			{ success = Importer::ImportMesh(buffer, (R_Mesh*)resource); }								break;
		case ResourceType::TEXTURE:			{ success = Importer::ImportTexture(buffer, read, (R_Texture*)resource); }					break;
		case ResourceType::SCENE:			{ /*success = HAVE A FUNCTIONAL R_SCENE AND LOAD/SAVE METHODS*/ }							break;
		case ResourceType::SHADER:			{ success = Importer::Shaders::Import(resource->GetAssetsPath(), (R_Shader*)resource); }	break;
		case ResourceType::PARTICLE_SYSTEM:	{ success = Importer::ImportParticles(buffer, (R_ParticleSystem*)resource); }				break;
		case ResourceType::SCRIPT:			{ success = Importer::Scripts::Import(assetsPath, buffer, read, (R_Script*)resource); }		break;
		case ResourceType::NAVMESH:			{ success = Importer::ImportNavMesh(buffer, (R_NavMesh*)resource); }						break;
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

void M_ResourceManager::DragAndDrop(const char* path)
{
	//Check if file is valid with extension
	std::string pathS, file,extension;
	App->fileSystem->SplitFilePath(path, &pathS, &file,&extension);

	std::string newPath = "Assets/";

	std::string fullFile = newPath + file + "." + extension;

	App->fileSystem->DuplicateFile(path, fullFile.c_str());

	//Import
	App->resourceManager->ImportFile(fullFile.c_str());
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

	if (extension == MODELS_EXTENSION)					{ type = ResourceType::MODEL; }
	else if (extension == MESHES_EXTENSION)				{ type = ResourceType::MESH; }
	else if (extension == MATERIALS_EXTENSION)			{ type = ResourceType::MATERIAL; }
	else if (extension == TEXTURES_EXTENSION)			{ type = ResourceType::TEXTURE; }
	else if (extension == FOLDERS_EXTENSION)			{ type = ResourceType::FOLDER; }
	else if (extension == SCENES_EXTENSION)				{ type = ResourceType::SCENE; }
	else if (extension == ANIMATIONS_EXTENSION)			{ type = ResourceType::ANIMATION; }
	else if (extension == SHADERS_EXTENSION)			{ type = ResourceType::SHADER; }
	else if (extension == PARTICLESYSTEMS_EXTENSION)	{ type = ResourceType::PARTICLE_SYSTEM; }
	else if (extension == SCRIPTS_EXTENSION)			{ type = ResourceType::SCRIPT; }
	else if (extension == NAVMESH_EXTENSION)			{ type = ResourceType::NAVMESH; }
	else												{ type = ResourceType::NONE; }

	return type;
}

// --- META FILE METHODS ---
bool M_ResourceManager::SaveMetaFile(Resource* resource) const
{
	if (resource == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not Save the Meta File! Error: Given Resource* was nullptr.");
		return false;
	}

	ParsonNode metaRoot = ParsonNode();
	metaRoot.SetNumber("UID", resource->GetUID());																											// --- GENERAL RESOURCE META DATA
	metaRoot.SetNumber("Type", (uint)resource->GetType());																									// 
	
	metaRoot.SetString("Name", resource->GetAssetsFile());																									// 
	metaRoot.SetString("AssetsPath", resource->GetAssetsPath());
	metaRoot.SetString("LibraryFile", resource->GetLibraryFile());
	metaRoot.SetString("LibraryPath", resource->GetLibraryPath());																							// 

	metaRoot.SetNumber("ModificationTime", (double)App->fileSystem->GetLastModTime(resource->GetAssetsPath()));												// ------------------------------

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

	return true;
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

	// TODO: Try to RELEASE the buffer here instead of having to do it every time the function is called wherever it is called.

	return ParsonNode(*buffer);
}

bool M_ResourceManager::HasMetaFile(const char* assetsPath)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not check whether or not the given path had an associated Meta File! Error: Given Assets Path was nullptr.");
		return false;
	}

	return App->fileSystem->Exists((assetsPath + std::string(META_EXTENSION)).c_str());
}

bool M_ResourceManager::MetaFileIsValid(const char* assetsPath, bool checkLibrary)
{
	if (assetsPath == nullptr)
	{
		LOG("[ERROR] Resource Manager: Could not validate Meta File! Error: Given Assets Path was nullptr.");
		return 0;
	}

	std::string metaPath		= assetsPath + std::string(META_EXTENSION);
	std::string errorString		= "[ERROR] Resource Manager: Could not validate Meta File " + metaPath;

	if (!App->fileSystem->Exists(metaPath.c_str()))
	{
		LOG("%s! Error: File System could not find the Meta File.", errorString.c_str());
		return false;
	}

	char* buffer					= nullptr;
	ParsonNode metaRoot				= LoadMetaFile(assetsPath, &buffer);
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

	std::string libraryPath		= metaRoot.GetString("LibraryPath");
	uint32 resourceUid			= (uint32)metaRoot.GetNumber("UID");
	if (!App->fileSystem->Exists(libraryPath.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", errorString.c_str());
		return false;
	}
	if (checkLibrary && (library.find(resourceUid) == library.end()))
	{
		LOG("%s! Error: Resource UID could not be found in Library.", errorString.c_str());
		return false;
	}

	ParsonNode containedNode			= ParsonNode();
	uint32 containedUid					= 0;
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
		if (checkLibrary && (library.find(containedUid) == library.end()))
		{
			LOG("%s! Error: Contained Resource UID could not be found in Library.", errorString.c_str());
			return false;
		}
	}

	return true;
}

bool M_ResourceManager::MetaFileIsValid(ParsonNode& metaRoot, bool checkLibrary)
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
	uint32 resourceUid			= (uint32)metaRoot.GetNumber("UID");
	if (!App->fileSystem->Exists(libraryPath.c_str()))
	{
		LOG("%s! Error: Resource Custom File could not be found in Library.", errorString.c_str());
		return false;
	}
	if (checkLibrary && (library.find(resourceUid) == library.end()))
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
		if (checkLibrary && (library.find(containedUid) == library.end()))
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
	
	switch (resource->GetType())
	{
		case ResourceType::FOLDER:			{ return true; }	break;
		case ResourceType::MODEL:			{ return true; }	break;
		case ResourceType::TEXTURE:			{ return true; }	break;
		case ResourceType::SHADER:			{ return true; }	break;
		case ResourceType::PARTICLE_SYSTEM:	{ return true; }	break;
		case ResourceType::SCRIPT:			{ return true; }	break;
		case ResourceType::NAVMESH:			{ return true; }	break;
	}

	return false;
}

bool M_ResourceManager::HasImportIgnoredExtension(const char* assetsPath) const
{
	std::string ext = App->fileSystem->GetFileExtension(assetsPath);

	return (ext == "ini" || ext == "json" || ext == "JSON" || ext == "txt" || ext == "ttf" || ext == "bnk" || ext == "prefab");
}