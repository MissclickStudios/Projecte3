#ifndef __M_RESOURCE_MANAGER_H__
#define __M_RESOURCE_MANAGER_H__

#include <map>
#include <string>

#include "Module.h"

class ParsonNode;
class Resource;

enum class ResourceType;
class R_Shader;
class R_Texture;

class GameObject;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

class NULL_API M_ResourceManager : public Module
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

	void			DragAndDrop(const char* path);

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
	
	const std::map<uint32, Resource*>* GetResources	() const;										// Returns a pointer to the resources map.

	R_Shader*		GetShader						(const char* name);								//Look for a shader in the library and load and return it
	void			GetAllShaders					(std::vector<R_Shader*>& shaders);				//Retrieve all the shaders in the library

	void			GetAllTextures					(std::vector<R_Texture*>& textures);			//Retrieve all the shaders in the library
	
	void			GetAllScripts					(std::map<std::string, std::string>& scripts);
	void			ReloadAllScripts				();												//Called when hot reloading the scripts
	

	// --- PREFAB METHODS
	void			CreatePrefab					(GameObject* gameObject);
	void			UpdatePrefab					(GameObject* gameObject);

	void			SavePrefab						(GameObject* gameObject, uint _prefabId);
	void			SavePrefabObject				(GameObject* gameObject, ParsonNode* node);
	void			LoadPrefab						(uint _prefabId);
	
private:																															// --- ASSETS MONITORING METHODS ---
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
	bool			GetLibraryDirectoryAndExtensionFromType		(const ResourceType& type, std::string& directory, std::string& extension);
	
	bool			LoadMetaLibraryPairsIntoLibrary				(const char* assetsPath);
	bool			GetLibraryPairsFromMeta						(const char* assetsPath, std::map<uint32, std::string>& pairs);

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
	std::map<uint32, std::string>	library;																		// UID and Library Path string of all loaded resources.

	float							fileRefreshTimer;																// 
	float							fileRefreshRate;																// 

public:
	std::map<uint32, std::string> prefabs;
};

#endif // !__M_RESOURCE_MANAGER_H__