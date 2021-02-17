#ifndef __M_RESOURCE_MANAGER_H__
#define __M_RESOURCE_MANAGER_H__

#include <map>
#include <string>

#include "Module.h"

class ParsonNode;
class Resource;

enum class ResourceType;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

class M_ResourceManager : public Module
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

public:																												// --- ASSETS MONITORING METHODS ---
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
	bool			GetLibraryFilePathsFromMeta					(const char* assetsPath, std::vector<std::string>& filePaths);
	bool			GetLibraryDirectoryAndExtensionFromType		(const ResourceType& type, std::string& directory, std::string& extension);
	
	bool			LoadMetaLibraryPairsIntoLibrary				(const char* assetsPath);
	bool			GetLibraryPairsFromMeta						(const char* assetsPath, std::map<uint32, std::string>& pairs);

	uint64			GetAssetFileModTimeFromMeta		(const char* assetsPath);

public:																												// --- IMPORT FILE METHODS ---
	uint32			ImportFile						(const char* assetsPath);										// 
	uint32			ImportFromAssets				(const char* assetsPath);										// 
	uint			SaveResourceToLibrary			(Resource* resource);											// 
	
	uint32			LoadFromLibrary					(const char* assetsPath);										// 

	const char*		GetValidPath					(const char* assetsPath);										// 
	ResourceType	GetTypeFromAssetsExtension		(const char* assetsPath);										// 
	ResourceType	GetTypeFromLibraryExtension		(const char* libraryPath);										// 

	void			SetResourceAssetsPathAndFile	(const char* assetsPath, Resource* resource);					// 
	void			SetResourceLibraryPathAndFile	(Resource* resource);											// 

public:																												// --- META FILE METHODS ---
	bool			SaveMetaFile					(Resource* resource) const;										//
	ParsonNode		LoadMetaFile					(const char* assetsPath, char** buffer);						// Passing the buffer so it can be safely RELEASED after calling it.
	
	bool			HasMetaFile						(const char* assetsPath);
	bool			MetaFileIsValid					(const char* assetsPath);
	bool			MetaFileIsValid					(ParsonNode& metaRoot);
	bool			ResourceHasMetaType				(Resource* resource) const;

	Resource*		GetResourceFromMetaFile			(const char* assetsPath);

public:																												// --- RESOURCE METHODS ---
	Resource*		CreateResource					(ResourceType type, const char* assetsPath = nullptr, const uint32& forcedUid = 0);	// 
	bool			DeleteResource					(const uint32& uid);																	//
	bool			DeleteResource					(Resource* resourceToDelete);															// FORCED DELETE
	void			GetResources					(std::map<uint32, Resource*>& resources) const;											// 
	
	Resource*		RequestResource					(const uint32& uid);																	// 
	bool			FreeResource					(const uint32& uid);																	// 
	
	Resource*		AllocateResource				(const uint32& uid, const char* assetsPath = nullptr);									// 
	bool			DeallocateResource				(const uint32& uid);																	// 
	bool			DeallocateResource				(Resource* resourceToDeallocate);														// FORCED DEALLOCATE
	
private:
	std::map<uint32, Resource*>		resources;																		// Resources currently in memory.
	std::map<uint32, std::string>	library;																		// UID and Library Path string of all loaded resources.

	float							fileRefreshTimer;																// 
	float							fileRefreshRate;																// 
};

#endif // !__M_RESOURCE_MANAGER_H__