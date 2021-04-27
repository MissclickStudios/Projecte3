#ifndef __M_RESOURCE_MANAGER_H__
#define __M_RESOURCE_MANAGER_H__

#include <map>
#include <string>

#include "Module.h"

#include "ResourceBase.h"
#include "Resource.h"

#include "Prefab.h"

class ParsonNode;

class Resource;
class R_Shader;
class R_Texture;
class R_ParticleSystem;

class GameObject;

//struct Prefab;

//enum class ResourceType;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

class MISSCLICK_API M_ResourceManager : public Module
{
public:
	M_ResourceManager();
	~M_ResourceManager();

	bool			Init				(ParsonNode& configuration) override;
	bool			Start				() override;
	UpdateStatus	PreUpdate			(float dt) override;
	UpdateStatus	Update				(float dt) override;
	UpdateStatus	PostUpdate			(float dt) override;
	bool			CleanUp				() override;

	bool			SaveConfiguration	(ParsonNode& configuration) const override;
	bool			LoadConfiguration	(ParsonNode& configuration) override;

public:																								// --- RESOURCE MANAGER API ---
	// --- IMPORT FILE METHODS
	uint32			ImportFile						(const char* assetsPath);						// Imports a file into the Library.
	uint			SaveResourceToLibrary			(Resource* resource);							// Saves a resource into the Library. Generates a custom file and a .meta file.
	uint32			LoadFromLibrary					(const char* assetsPath);						// Loads a resource registered in the Library onto memory. Returns the resource's UID.
	Resource*		GetResourceFromLibrary			(const char* assetsPath);						// Same as LoadFromLibrary() but it returns the resource instead of its UID.

	void			DragAndDrop(const char* path);													// 

	void			RefreshProjectDirectories		();												// 
	void			RefreshProjectDirectory			(const char* directoryToRefresh);				// 

	// --- META FILE METHODS
	ResourceType	GetTypeFromAssetsExtension		(const char* assetsPath);						// Returns the type of the resource related with the given Asset.
	uint32			GetForcedUIDFromMeta			(const char* assetsPath);
	bool			GetForcedUIDsFromMeta			(const char* assetsPath, std::map<std::string, uint32>& forcedUIDs);			// TMP. See if it can be done another way.
																																	// Gets the UIDs stored in a .meta and assets path.
	// --- RESOURCE METHODS
	Resource*		CreateResource					(ResourceType type, const char* assetsPath = nullptr, uint32 forcedUID = 0);	// Generates a rsrc with the spec. params. No emplace.
	Resource*		RequestResource					(uint32 UID);									// Returns an active resource with the given UID. Increases the rsrc's references by 1.
	bool			AllocateResource				(uint32 UID, const char* assetsPath = nullptr);	// Loads the rsrc with the given UID onto memory. LoadFromLibrary() calls this.
	
	bool			FreeResource					(uint32 UID);									// Reduces the ref. count of the rsrc with the given UID by 1. If refs = 0, it is dealloc.
	bool			DeallocateResource				(uint32 UID);									// Deletes an active resource from memory. It still remains in the Library.
	bool			DeallocateResource				(Resource* resourceToDeallocate);				// Same as the above but directly passing the resource as the argument.
	bool			DeleteResource					(uint32 UID);									// Completely erases a resource, both from memory and from the library.
	bool			DeleteResource					(Resource* resourceToDelete);					// Same as the above but directly passing the resource as the argument.
	
	bool			RefreshResourceAssets			(ResourceType type);

	const std::map<uint32, Resource*>* GetResourcesMap	() const;									// Returns a pointer to the resources map.

	template <typename T>
	T* GetResource(uint32 UID)
	{
		return (UID != 0) ? (T*)RequestResource(UID) : nullptr;										// Check for return type mismatch?
	}
	
	template <typename T>
	T* GetResource(const char* assetsPath)
	{
		if (assetsPath == nullptr)
			return nullptr;

		uint32 UID = LoadFromLibrary(assetsPath);
		if (UID != 0)
		{
			return (T*)RequestResource(UID);
		}

		return nullptr;
	}

	template <typename T>
	bool GetResources(std::vector<T*>& resourcesWithType) const											// Remember to free the resources after finishing working with the vector.
	{
		if (resources.empty())
			return false;

		for (auto resource = resources.cbegin(); resource != resources.cend(); ++resource)
		{
			if (resource->second->GetType() == T::GetType())
			{
				resourcesWithType.push_back((T*)resource->second);										// Request here? Leave it to the user? bool argument?
				++(*resourcesWithType)->references;
			}
		}
		
		return !resourcesWithType.empty();
	}

	template <typename T>
	bool GetResourcesFromLibrary(std::vector<T*>& resourcesWithType)
	{
		if (library.empty())
			return false;

		for (auto libItem = library.cbegin(); libItem != library.cend(); ++libItem)
		{
			if (libItem->second.type == T::GetType())
			{
				if (AllocateResource(libItem->second.UID, libItem->second.assetsPath.c_str()))
				{
					T* resource = (T*)RequestResource(libItem->second.UID);
					if (resource != nullptr)
					{
						resourcesWithType.push_back(resource);
					}
				}
			}
		}
		
		return !resourcesWithType.empty();
	}

	template <typename T>
	bool GetResourceBases(std::vector<ResourceBase>& resourceBases)
	{
		if (library.empty())
			return false;

		for (auto libItem = library.cbegin(); libItem != library.cend(); ++libItem)
		{
			if (libItem->second.type == T::GetType())
			{
				resourceBases.push_back(libItem->second);
			}
		}
	}

	R_Shader*		GetShader						(const char* name);															// Look for a shader in the library and load and return it
	void			GetAllShaders					(std::vector<R_Shader*>& shaders);											// Retrieve all the shaders in the library
	void			GetAllParticleSystems			(std::vector<R_ParticleSystem*>& shaders);									// Retrieve all the particlesystems in the library

	void			GetAllTextures					(std::vector<R_Texture*>& textures, const char* name = nullptr);			// Retrieve all the shaders in the library

	void			GetAllScripts					(std::map<std::string, std::string>& scripts);								// 
	void			ReloadAllScripts				();																			// Called when hot reloading the scripts
	

	// --- PREFAB METHODS
	void			CreatePrefab					(GameObject* gameObject);
	void			UpdatePrefab					(GameObject* gameObject);

	void			SavePrefab						(GameObject* gameObject, uint _prefabId);
	void			SavePrefabObject				(GameObject* gameObject, ParsonNode* node);
	GameObject*		LoadPrefab						(uint _prefabId, GameObject* parent, GameObject* rootObject = nullptr);		// If the root object isn't NULL its Trfrm Comp. will be used.

	Prefab*			GetPrefab(uint uid);
	const char*		GetPrefabName(uint uid);
	Prefab*			GetPrefabByName(const char* prefabName);
	uint			GetPrefabUIDByName(const char* prefabName);
	
private:																														// --- ASSETS MONITORING METHODS ---
	void			RefreshDirectoryFiles			(const char* directory);
	void			RefreshDirectory				(const char* directory, std::vector<std::string>& filesToImport, 
														std::vector<std::string>& filesToUpdate, std::vector<std::string>& filesToDelete);
	
	void			FindFilesToImport				(const std::vector<std::string>& assetFiles, const std::vector<std::string>& metaFiles, 
														std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToImport);
	
	void			FindFilesToUpdate				(const std::map<std::string, std::string>& filePairs, std::vector<std::string>& filesToUpdate);

	void			FindFilesToDelete				(const std::vector<std::string>& metaFiles, const std::map<std::string, std::string>& filePairs, 
														std::vector<std::string>& filesToDelete);

	void			LoadValidFilesIntoLibrary		(const std::map<std::string, std::string>& filePairs);

	bool			DeleteFromAssets				(const char* assetsPath);
	bool			DeleteFromLibrary				(const char* assetsPath);

	bool			GetResourceUIDsFromMeta						(const char* assetsPath, std::vector<uint32>& resourceUids);
	//bool			GetForcedUIDsFromMeta						(const char* assetsPath, std::map<std::string, uint32>& forcedUIDs);
	bool			GetLibraryFilePathsFromMeta					(const char* assetsPath, std::vector<std::string>& filePaths);
	bool			GetResourceBasesFromMeta					(const char* assetsPath, std::vector<ResourceBase>& resourceBases);
	bool			GetAssetsDirectoryAndExtensionFromType		(const ResourceType& type, std::string& directory, std::string& extension);
	bool			GetLibraryDirectoryAndExtensionFromType		(const ResourceType& type, std::string& directory, std::string& extension);
	
	bool			LoadMetaLibraryPairsIntoLibrary				(const char* assetsPath);
	bool			GetLibraryPairsFromMeta						(const char* assetsPath, std::map<uint32, ResourceBase>& pairs);

	uint64			GetAssetFileModTimeFromMeta					(const char* assetsPath);
	
	void			FindPrefabs						();																//Finds all prefabs in Assets/Prefabs
	
private:																											// --- IMPORT FILE METHODS ---
	uint32			ImportFromAssets				(const char* assetsPath);										// 

	
	const char*		GetValidPath					(const char* assetsPath);										// 
	ResourceType	GetTypeFromLibraryExtension		(const char* libraryPath);										// 

private:																											// --- META FILE METHODS ---
	bool			SaveMetaFile					(Resource* resource) const;										//
	ParsonNode		LoadMetaFile					(const char* assetsPath, char** buffer);						// Passing the buffer so it can be safely RELEASED after calling it.
	
	bool			HasMetaFile						(const char* assetsPath);
	bool			MetaFileIsValid					(const char* assetsPath, bool checkLibrary = true);
	bool			MetaFileIsValid					(ParsonNode& metaRoot, bool checkLibrary = true);
	bool			ResourceHasMetaType				(Resource* resource) const;

	bool			HasImportIgnoredExtension		(const char* assetsPath) const;

private:
	std::map<uint32, Resource*>		resources;																		// Resources currently in memory.
	std::map<uint32, ResourceBase>	library;																		// UID and Library Path string of all loaded resources.

	float							fileRefreshTimer;																// 
	float							fileRefreshRate;																// 

public:
	std::map<uint32, Prefab> prefabs;
};

#endif // !__M_RESOURCE_MANAGER_H__